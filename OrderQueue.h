#pragma once

#include "Common.h"
#include "Order.h"

// One node of the pending-order queue. It only points to the order,
// the order itself is owned by OrderManager.
class QueueNode {
public:
    Order* order;
    QueueNode* next;    // next node toward the rear

    QueueNode(Order* order);
};

// Maintains front/rear links and a node count; Order objects are managed elsewhere.
class OrderQueue {
private:
    QueueNode* front;   // oldest pending order
    QueueNode* rear;    // newest pending order
    int size;

public:
    OrderQueue();
    ~OrderQueue();      // deletes nodes only, never the orders

    // The queue holds raw node pointers, so copying is not allowed
    OrderQueue(const OrderQueue&) = delete;
    OrderQueue& operator=(const OrderQueue&) = delete;

    // Add at rear / remove from front; dequeue returns nullptr when empty.
    void enqueue(Order* order);
    Order* dequeue();
    Order* peek() const;

    // Remove by normalized ID, reinsert at front, or remove cancelled entries.
    bool removeById(string orderId);
    void pushFront(Order* order);
    int removeCancelled();

    void display() const;

    // Accessors expose the head link and queue state without modifying it.
    QueueNode* getFrontNode() const;
    bool isEmpty() const;
    int getSize() const;
};
