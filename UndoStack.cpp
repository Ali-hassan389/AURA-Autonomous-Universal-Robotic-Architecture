#include "UndoStack.h"

using namespace std;
// Initialize fields for the record variants and clear the link pointer.
// Every field starts with a safe default value
UndoRecord::UndoRecord()
{
	type = QTY_UPDATE;

	productId = "";
	rackId = "";
	oldQty = 0;
	newQty = 0;

	pathA = "";
	pathB = "";
	oldPathStatus = Path_Open;

	robotId = "";
	oldRobotStatus = Robot_Available;

	next = nullptr;
}


// UNDO STACK 

UndoStack::UndoStack()
	: top(nullptr), size(0)
{
}


// Pops and deletes every record that is still on the stack
UndoStack::~UndoStack()
{
	while (top != nullptr)
	{
		UndoRecord* removed = pop();
		delete removed;
	}
}


// ==================== PUSH ====================

// New record points to the old top, then becomes the new top
void UndoStack::push(UndoRecord* record)
{
	if (record == nullptr)
		return;

	record->next = top;
	top = record;
	size++;
}


//  POP 

// Unlinks the top record and hands it to the caller.
// The caller is responsible for deleting it.
UndoRecord* UndoStack::pop()
{
	if (top == nullptr)
		return nullptr;

	UndoRecord* removed = top;

	top = top->next;
	removed->next = nullptr;
	size--;

	return removed;
}


// PEEK 

// Return the top pointer without changing stack contents.
UndoRecord* UndoStack::peek() const
{
	return top;
}


// = ACCESSORS 

bool UndoStack::isEmpty() const
{
	return top == nullptr;
}


int UndoStack::getSize() const
{
	return size;
}
