#pragma once

#include "Common.h"

class InventoryManager;

// One product line in an order; next links lines in insertion order.
class OrderItem {
public:
    string productId;
    int quantity;
    int deliveredQty;
    OrderItem* next;

    OrderItem(string productId, int quantity);
};

// Order record with item list, status, and optional robot assignment.
class Order {
public:
    string orderId;
    string customerId;
    OrderItem* itemHead;
    OrderStatus status;
    string assignedRobotId;
    bool inQueue;
    Order* next;

    Order(string orderId, string customerId);
    ~Order();

    Order(const Order&) = delete;
    Order& operator=(const Order&) = delete;

    // Add quantity to a product line or append a new line; rejects invalid input.
    bool addItem(string productId, int qty);
    // Find a product line by identifier, or return nullptr.
    OrderItem* findItem(string productId);

    // Return quantity not yet delivered, bounded below by zero.
    int remainingQty(const OrderItem* item) const;
    // Calculate remaining weight using unit weights from inventory.
    double totalWeight(InventoryManager& inventory);
    // Return the highest unit weight among products still remaining.
    double heaviestUnitWeight(InventoryManager& inventory);

    // Test whether a nonempty order has all line quantities delivered.
    bool isFullyDelivered() const;
    // Test whether any line has a recorded delivery.
    bool hasAnyDelivery() const;

    // Print this order and its item quantities.
    void displayOrder() const;
};
