#pragma once

#include "common.h"

// InventoryManager is only needed inside Order.cpp (to read unit weights),
// so a forward declaration is enough here and avoids circular includes.
class InventoryManager;

// One line of an order: a product, how many were ordered and how many
// have already been delivered to the packing station.
class OrderItem {
public:
    string productId;
    int quantity;       // units ordered
    int deliveredQty;   // units already delivered
    OrderItem* next;    // next item in the order's item list

    OrderItem(string productId, int quantity);
};

// One customer order. It owns a singly linked list of OrderItem nodes.
// The order itself lives in OrderManager's main list (via the next pointer).
class Order {
public:
    string orderId;
    string customerId;
    OrderItem* itemHead;        // head of the item list
    OrderStatus status;
    string assignedRobotId;     // empty when no robot is assigned
    bool inQueue;               // true while waiting in the pending queue
    Order* next;                // next order in the main order list

    // Constructor and destructor
    Order(string orderId, string customerId);
    ~Order();

    // The order owns a linked list, so copying is disabled to avoid
    // two orders deleting the same item nodes.
    Order(const Order&) = delete;
    Order& operator=(const Order&) = delete;

    // Item list operations
    bool addItem(string productId, int qty);
    OrderItem* findItem(string productId);

    // Quantity and weight calculations
    int remainingQty(const OrderItem* item) const;
    double totalWeight(InventoryManager& inventory);
    double heaviestUnitWeight(InventoryManager& inventory);

    // Delivery state checks
    bool isFullyDelivered() const;
    bool hasAnyDelivery() const;

    // Output
    void displayOrder() const;
};
