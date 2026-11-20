#pragma once

#include "Common.h"

// The three administrative actions that can be undone.
enum UndoType {
    QTY_UPDATE,
    PATH_BLOCK,
    ROBOT_MAINTENANCE
};

// One history entry; fields are used according to its UndoType discriminator.
class UndoRecord {
public:
    UndoType type;

    // Quantity-update payload.
    string productId;
    string rackId;
    int oldQty;
    int newQty;

    // Pathway-block payload.
    string pathA;
    string pathB;
    PathStatus oldPathStatus;

    // Robot-maintenance payload.
    string robotId;
    RobotStatus oldRobotStatus;

    UndoRecord* next;

    UndoRecord();
};

// Singly linked LIFO stack. push, pop, and peek are O(1).
class UndoStack {
private:
    UndoRecord* top;
    int size;

public:
    UndoStack();
    ~UndoStack();

    UndoStack(const UndoStack&) = delete;
    UndoStack& operator=(const UndoStack&) = delete;

    // Stack operations: push links a record, pop transfers the top pointer, peek observes it.
    void push(UndoRecord* record);
    UndoRecord* pop();
    UndoRecord* peek() const;
    bool isEmpty() const;
    int getSize() const;
};
