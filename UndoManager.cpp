#include "UndoManager.h"

#include <iostream>

using namespace std;


// Keep references to the collaborating warehouse subsystems and initialize empty history.
// Constructor
UndoManager::UndoManager(Graph& warehouseGraph,InventoryManager& inventoryManager,RobotManager& robotManager)
	: graph(warehouseGraph),
	inventory(inventoryManager),
	robots(robotManager) {
}


// Store the values needed to restore a manual quantity change.
// Record a manual quantity update
void UndoManager::recordQuantityUpdate(
	const string& productId,
	const string& rackId,
	int oldQty,
	int newQty) {

	UndoRecord* record = new UndoRecord();

	record->type = QTY_UPDATE;
	record->productId = productId;
	record->rackId = rackId;
	record->oldQty = oldQty;
	record->newQty = newQty;

	history.push(record);

	cout << "Quantity update added to undo history.\n";
}


// Store the pathway endpoints and the status that preceded the block.
// Record a blocked pathway
void UndoManager::recordBlock(
	const string& pathA,
	const string& pathB,
	PathStatus oldStatus) {

	UndoRecord* record = new UndoRecord();

	record->type = PATH_BLOCK;
	record->pathA = pathA;
	record->pathB = pathB;
	record->oldPathStatus = oldStatus;

	history.push(record);

	cout << "Pathway block added to undo history.\n";
}


// Store the robot ID and its previous status for a potential restoration.
// Record a robot maintenance change
void UndoManager::recordMaintenance(
	const string& robotId,
	RobotStatus oldStatus) {

	UndoRecord* record = new UndoRecord();

	record->type = ROBOT_MAINTENANCE;
	record->robotId = robotId;
	record->oldRobotStatus = oldStatus;

	history.push(record);

	cout << "Maintenance change added to undo history.\n";
}


// Display the most recent record without removing it
void UndoManager::displayMostRecent() {

	if (history.isEmpty()) {
		cout << "\nUndo history is empty.\n";
		return;
	}

	UndoRecord* record = history.peek();

	cout << "\n========== MOST RECENT ACTION ==========\n";

	if (record->type == QTY_UPDATE) {

		cout << "Action: Manual quantity update\n";
		cout << "Product: " << record->productId << '\n';
		cout << "Rack: " << record->rackId << '\n';
		cout << "Changed from: " << record->oldQty << '\n';
		cout << "Changed to: " << record->newQty << '\n';

	}
	else if (record->type == PATH_BLOCK) {

		cout << "Action: Block pathway\n";
		cout << "Pathway: " << record->pathA
			<< " <-> " << record->pathB << '\n';
		cout << "Previous status: "
			<< static_cast<int>(record->oldPathStatus) << '\n';

	}
	else if (record->type == ROBOT_MAINTENANCE) {

		cout << "Action: Robot maintenance change\n";
		cout << "Robot ID: " << record->robotId << '\n';
		cout << "Previous status: "
			<< static_cast<int>(record->oldRobotStatus) << '\n';
	}

	cout << "========================================\n";
}


// Validate and restore the top record; failed validations generally leave it in history.
// Undo the most recent action
bool UndoManager::undoLast() {

	if (history.isEmpty()) {
		cout << "\nNothing to undo.\n";
		return false;
	}

	UndoRecord* record = history.peek();

	// Quantity undo verifies the rack and reservation constraints before restoring the old value.
	// CASE 1: Quantity update
	if (record->type == QTY_UPDATE) {

		// Check whether the rack entry still exists
		if (!inventory.rackEntryExists(
			record->productId, record->rackId)) {

			cout << "Undo rejected: rack entry no longer exists.\n";
			return false;
		}

		// Restoring quantity must not go below reserved quantity
		int reserved = inventory.reservedQty(
			record->productId, record->rackId);

		if (record->oldQty < reserved) {

			cout << "Undo rejected: old quantity is below "
				<< "the currently reserved quantity.\n";

			return false;
		}

		int ignoredOldValue;

		if (!inventory.setQuantity(
			record->productId,
			record->rackId,
			record->oldQty,
			ignoredOldValue)) {

			cout << "Undo rejected: quantity could not be restored.\n";
			return false;
		}

		cout << "Quantity restored from "
			<< record->newQty << " to "
			<< record->oldQty << ".\n";
	}


	// Pathway undo attempts the saved status restoration; this branch consumes the record either way.
	// CASE 2: Blocked pathway
	else if (record->type == PATH_BLOCK) {

		bool restored = graph.setPathwayStatus(
			record->pathA,
			record->pathB,
			record->oldPathStatus);

		if (!restored) {
			cout << "Pathway no longer exists. "
				<< "Removing its undo record.\n";
		}
		else {
			cout << "Pathway status restored.\n";
		}

		// Guide says this record is popped even if pathway is missing
		UndoRecord* removed = history.pop();
		delete removed;

		return true;
	}


	// Robot undo requires an existing robot that is not in one of the listed active states.
	// CASE 3: Robot maintenance
	else if (record->type == ROBOT_MAINTENANCE) {

		Robot* robot = robots.findRobot(record->robotId);

		if (robot == nullptr) {
			cout << "Undo rejected: robot no longer exists.\n";
			return false;
		}

		RobotStatus currentStatus = robot->getStatus();

		// Do not undo if the robot is currently busy or charging
		if (currentStatus == Robot_Assigned ||
			currentStatus == Robot_Charging ||
			currentStatus == Robot_WaitingforCharging) {

			cout << "Undo rejected: robot is assigned, "
				<< "charging, or waiting for charging.\n";

			return false;
		}

		if (!robots.restoreStatus(
			record->robotId,
			record->oldRobotStatus)) {

			cout << "Undo rejected: robot status could not be restored.\n";
			return false;
		}

		cout << "Robot maintenance status restored.\n";
	}


	// Remove the record only after successful restoration
	UndoRecord* removed = history.pop();
	delete removed;

	cout << "Undo completed successfully.\n";

	return true;
}


// Discard the latest record without undoing it
bool UndoManager::discardTop() {

	if (history.isEmpty()) {
		cout << "\nNothing to discard.\n";
		return false;
	}

	UndoRecord* removed = history.pop();
	delete removed;

	cout << "Most recent undo record discarded.\n";

	return true;
}


// Check whether history is empty
bool UndoManager::isEmpty() const {
	return history.isEmpty();
}


// Interactive console menu for viewing, undoing, or discarding the top history entry.
// Undo menu
void runUndoMenu(UndoManager& undo) {

	int choice;

	do {
		cout << "\n========== HISTORY AND UNDO ==========\n";
		cout << "1. Show most recent action\n";
		cout << "2. Undo last action\n";
		cout << "3. Discard most recent action\n";
		cout << "0. Back\n";

		choice = readIntInRange("Enter choice: ", 0, 3);

		switch (choice) {

		case 1:
			undo.displayMostRecent();
			break;

		case 2:
			undo.undoLast();
			break;

		case 3:
			undo.discardTop();
			break;

		case 0:
			cout << "Returning to main menu...\n";
			break;
		}

	} while (choice != 0);
}
