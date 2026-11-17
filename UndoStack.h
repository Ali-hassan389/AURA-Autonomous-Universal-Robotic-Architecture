#pragma once

#include "Common.h"

// The three administrative actions that can be undone
enum UndoType {
	QTY_UPDATE,
	PATH_BLOCK,
	ROBOT_MAINTENANCE
};


// One node of the undo stack. Only the fields that belong to the
// record's type are used, the rest keep their default values.
class UndoRecord {
public:
	UndoType type;

	// Manual quantity update
	string productId;
	string rackId;
	int oldQty;
	int newQty;

	// Blocked pathway
	string pathA;
	string pathB;
	PathStatus oldPathStatus;

	// Robot maintenance change
	string robotId;
	RobotStatus oldRobotStatus;

	// Record below this one on the stack
	UndoRecord* next;

	UndoRecord();
};


// Stack implemented as a singly linked list with a top pointer.
// push, pop and peek are all O(1).
class UndoStack {
private:
	UndoRecord* top;
	int size;

public:
	UndoStack();
	~UndoStack();

	// Prevent shallow copying of the linked list
	UndoStack(const UndoStack&) = delete;
	UndoStack& operator=(const UndoStack&) = delete;

	// Puts a record on top of the stack
	void push(UndoRecord* record);

	// Unlinks the top record and returns it, the caller deletes it
	UndoRecord* pop();

	// Returns the top record without removing it
	UndoRecord* peek() const;

	bool isEmpty() const;
	int getSize() const;
};
