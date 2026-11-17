#pragma once

#include "Graph.h"
#include "InventoryManager.h"
#include "OrderManager.h"
#include "PickingPlan.h"
#include "PlanManager.h"
#include "RobotManager.h"

// Executes the picking plans made by PlanManager, one trip at a time.
// Every trip follows "validate first, then apply".
// This class owns no memory, it only changes data owned by other managers.
class ExecutionManager {
private:
	PlanManager& planner;
	Graph& graph;
	InventoryManager& inventory;
	OrderManager& orders;
	RobotManager& robots;

	// Prints the plan for one order with the next trip in detail
	void displayPlanForExecution(PickingPlan* plan);

	// Called after the last trip: frees the robot and discards the plan
	void finishOrderIfDone(const string& orderId);

public:
	ExecutionManager(PlanManager& planManager,
		Graph& warehouseGraph,
		InventoryManager& inventoryManager,
		OrderManager& orderManager,
		RobotManager& robotManager);

	// Prevent copying because this class stores references
	ExecutionManager(const ExecutionManager&) = delete;
	ExecutionManager& operator=(const ExecutionManager&) = delete;

	// Lists every order that has an active plan, returns how many
	int displayAssignedOrders();

	// Screen for one order: execute, recalculate, cancel or leave
	void executeTaskMenu(const string& orderId);

	// First trip of the plan that is not executed yet
	PlanTrip* getNextTrip(PickingPlan* plan);

	// Read-only checks, changes nothing
	bool validateTrip(Order* order, Robot* robot,
		PlanTrip* trip, string& reason);

	// Validates, then collects, moves the robot and records delivery
	bool executeTrip(const string& orderId);

	// Recomputes the routes of the unexecuted trips
	bool recalculateRoute(const string& orderId);

	// Releases reservations and robot, order goes back to the queue
	bool cancelAssignment(const string& orderId);
};


// Menu is a free function, not a class member
void runExecutionMenu(ExecutionManager& executor); #pragma once
