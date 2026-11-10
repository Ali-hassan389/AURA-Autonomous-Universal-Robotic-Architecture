#pragma once

#include "common.h"
#include "Order.h"

// One node of the pending-order queue. It only points to the order,
// the order itself is owned by OrderManager.
class QueueNode {
public:
    Order* order;
    QueueNode* next;    // next node toward the rear

    QueueNode(Order* order);
};

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

    void enqueue(Order* order);
    Order* dequeue();
    Order* peek() const;

    bool removeById(string orderId);
    void pushFront(Order* order);
    int removeCancelled();

    void display() const;

    QueueNode* getFrontNode() const;
    bool isEmpty() const;
    int getSize() const;
};
