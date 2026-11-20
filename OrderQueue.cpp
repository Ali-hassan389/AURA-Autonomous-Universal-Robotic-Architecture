#include "OrderQueue.h"

// A queue node stores a non-owning pointer to an order and a link to the next node.
// Creates a node that points to the given order
QueueNode::QueueNode(Order* order) {
    this->order = order;
    next = nullptr;
}
// Starts with an empty queue
OrderQueue::OrderQueue() {
    front = nullptr;
    rear = nullptr;
    size = 0;
}

// Deletes only the nodes. The orders belong to OrderManager.
OrderQueue::~OrderQueue() {
    QueueNode* current = front;
    while (current != nullptr) {
        QueueNode* nextNode = current->next;
        delete current;
        current = nextNode;
    }
    front = nullptr;
    rear = nullptr;
    size = 0;
}

// Adds an order at the rear of the queue and marks it as queued
void OrderQueue::enqueue(Order* order) {
    if (order == nullptr) {
        return;
    }

    // Link the new node as both endpoints for the empty case, otherwise append it.
    QueueNode* newNode = new QueueNode(order);
    if (rear == nullptr) {
        // queue was empty, so the new node is both front and rear
        front = newNode;
        rear = newNode;
    }
    else {
        rear->next = newNode;
        rear = newNode;
    }
    order->inQueue = true;
    size++;
}

// Removes the front node and returns its order (nullptr if queue is empty)
Order* OrderQueue::dequeue() {
    if (front == nullptr) {
        return nullptr;
    }

    // Save the returned order, unlink and delete only its queue node.
    QueueNode* oldFront = front;
    Order* result = oldFront->order;

    front = front->next;
    if (front == nullptr) {
        // queue became empty, so rear must be cleared too
        rear = nullptr;
    }
    delete oldFront;
    size--;

    result->inQueue = false;
    return result;
}

// Returns the front order without removing it
Order* OrderQueue::peek() const {
    if (front == nullptr) {
        return nullptr;
    }
    return front->order;
}

// Finds an order by ID anywhere in the queue and unlinks it. O(n).
// Returns true if it was found and removed.
bool OrderQueue::removeById(string orderId) {
    // Normalize the requested ID, then scan while tracking the predecessor for unlinking.
    string cleanOrderId = toUpperCase(trimText(orderId));
    QueueNode* previous = nullptr;
    QueueNode* current = front;

    while (current != nullptr) {
        if (current->order->orderId == cleanOrderId) {
            if (previous == nullptr) {
                // removing the front node
                front = current->next;
            }
            else {
                previous->next = current->next;
            }
            if (current == rear) {
                // removing the rear node, so rear moves back one step
                rear = previous;
            }
            current->order->inQueue = false;
            delete current;
            size--;
            return true;
        }
        previous = current;
        current = current->next;
    }
    return false;
}

// Puts an order back at the front so it becomes the oldest again
// (used by Cancel Assignment)
void OrderQueue::pushFront(Order* order) {
    if (order == nullptr) {
        return;
    }

    QueueNode* newNode = new QueueNode(order);
    newNode->next = front;
    front = newNode;
    if (rear == nullptr) {
        // queue was empty
        rear = newNode;
    }
    order->inQueue = true;
    size++;
}

// Unlinks every order whose status is Cancelled. Returns how many were removed.
int OrderQueue::removeCancelled() {
    // Track the predecessor while removing matching nodes; retain it when a node is deleted.
    int removed = 0;
    QueueNode* previous = nullptr;
    QueueNode* current = front;

    while (current != nullptr) {
        QueueNode* nextNode = current->next;

        if (current->order->status == Order_Cancelled) {
            if (previous == nullptr) {
                front = nextNode;
            }
            else {
                previous->next = nextNode;
            }
            if (current == rear) {
                rear = previous;
            }
            current->order->inQueue = false;
            delete current;
            size--;
            removed++;
            // previous stays the same because current was removed
        }
        else {
            previous = current;
        }
        current = nextNode;
    }
    return removed;
}

// Prints the queue from front to rear without changing it
void OrderQueue::display() const {
    if (front == nullptr) {
        cout << "Pending queue is empty." << endl;
        return;
    }

    // Traverse in FIFO order and delegate each order’s printed details to Order.
    cout << "Pending orders (" << size << "), front to rear:" << endl;
    QueueNode* current = front;
    int position = 1;
    while (current != nullptr) {
        cout << position << ". ";
        current->order->displayOrder();
        current = current->next;
        position++;
    }
}

// Returns the front node so the planner can walk the queue using next
QueueNode* OrderQueue::getFrontNode() const {
    return front;
}

bool OrderQueue::isEmpty() const {
    return front == nullptr;
}

int OrderQueue::getSize() const {
    return size;
}
