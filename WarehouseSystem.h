#pragma once
#include "Graph.h"
#include "RackRegistry.h"
#include "InventoryManager.h"
#include "OrderManager.h"
#include "RobotManager.h"
#include "UndoManager.h"
#include "PlanManager.h"
#include "ExecutionManager.h"
#include "ChargingManager.h"

class WarehouseSystem {
private:
    Graph graph;
    RackRegistry racks;
    InventoryManager inventory;
    OrderManager orders;
    RobotManager robots;
    UndoManager undo;
    PlanManager planner;
    ExecutionManager executor;
    ChargingManager charging;

    bool dataLoaded;

public:
    WarehouseSystem();
    WarehouseSystem(const WarehouseSystem&) = delete;
    WarehouseSystem& operator=(const WarehouseSystem&) = delete;

    bool loadData(const string& folder = ".");
    void showWelcome() const;
    void run();
};
