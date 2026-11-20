#pragma once

#include "Graph.h"
#include "InventoryManager.h"
#include "OrderManager.h"
#include "PickingPlan.h"
#include "PlanManager.h"
#include "RobotManager.h"

// Execution interface for running and managing assigned warehouse picking plans.
// Executes the picking plans made by PlanManager, one trip at a time.
// Every trip follows validate first, then apply.
// This class owns no memory; it changes data owned by other managers.
class ExecutionManager {
private:
    // Non-owning references to the managers and graph used during execution.
    PlanManager& planner;
    Graph& graph;
    InventoryManager& inventory;
    OrderManager& orders;
    RobotManager& robots;

    // Private presentation/helper operations used by the task menu.
    void displayPlanForExecution(PickingPlan* plan);
    void finishOrderIfDone(const string& orderId);

public:
    ExecutionManager(PlanManager& planManager,
                     Graph& warehouseGraph,
                     InventoryManager& inventoryManager,
                     OrderManager& orderManager,
                     RobotManager& robotManager);

    // Prevent copying a manager that holds references to shared system state.
    ExecutionManager(const ExecutionManager&) = delete;
    ExecutionManager& operator=(const ExecutionManager&) = delete;

    // Show active plans; return the number displayed.
    int displayAssignedOrders();
    void executeTaskMenu(const string& orderId);
    PlanTrip* getNextTrip(PickingPlan* plan);
    // Validate preconditions and return the first failure explanation via reason.
    bool validateTrip(Order* order, Robot* robot,
                      PlanTrip* trip, string& reason);
    bool executeTrip(const string& orderId);
    bool recalculateRoute(const string& orderId);
    bool cancelAssignment(const string& orderId);
};

// Entry point for the interactive execution menu.
void runExecutionMenu(ExecutionManager& executor);
