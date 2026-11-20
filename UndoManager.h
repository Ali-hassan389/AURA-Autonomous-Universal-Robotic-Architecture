#pragma once

#include "UndoStack.h"
#include "Graph.h"
#include "InventoryManager.h"
#include "RobotManager.h"

// Owns the undo stack and borrows subsystem managers used to restore recorded state.
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

    UndoManager(const UndoManager&) = delete;
    UndoManager& operator=(const UndoManager&) = delete;

    // Recording methods save the prior values needed by undoLast().
    void recordQuantityUpdate(const string& productId,
                              const string& rackId,
                              int oldQty,
                              int newQty);

    void recordBlock(const string& pathA,
                     const string& pathB,
                     PathStatus oldStatus);

    void recordMaintenance(const string& robotId,
                           RobotStatus oldStatus);

    // Inspect history, undo its top entry, discard it, or query whether history is empty.
    void displayMostRecent();
    bool undoLast();
    bool discardTop();
    bool isEmpty() const;
};

// Run the history/undo console interface until the user chooses Back.
void runUndoMenu(UndoManager& undo);
