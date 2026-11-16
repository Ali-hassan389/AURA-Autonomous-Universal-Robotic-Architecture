
#pragma once

#include "UndoStack.h"
#include "Graph.h"
#include "InventoryManager.h"
#include "RobotManager.h"

class UndoManager {
private:
	UndoStack history;

	Graph& graph;
	InventoryManager& inventory;
	RobotManager& robots;

public:
	UndoManager(Graph& warehouseGraph,
		InventoryManager& inventoryManager,
		RobotManager& robotManager);

	// Prevent copying because this class stores references
	UndoManager(const UndoManager&) = delete;
	UndoManager& operator=(const UndoManager&) = delete;

	// Record reversible operations
	void recordQuantityUpdate(const string& productId,
		const string& rackId,
		int oldQty,
		int newQty);

	void recordBlock(const string& pathA,
		const string& pathB,
		PathStatus oldStatus);

	void recordMaintenance(const string& robotId,
		RobotStatus oldStatus);

	// History operations
	void displayMostRecent();
	bool undoLast();
	bool discardTop();

	bool isEmpty() const;
};

void runUndoMenu(UndoManager& undo);


