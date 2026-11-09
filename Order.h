#pragma once

#include "Common.h"

class InventoryManager;

class OrderItem {
public:
    string productId;
    int quantity;       // units ordered
    int deliveredQty;   // units already delivered
    OrderItem* next;    // next item in the order's item list

    OrderItem(string productId, int quantity);
};

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

    // Item list operations
    bool addItem(string productId, int qty);
    OrderItem* findItem(string productId);

    // Quantity and weight calculations
    int remainingQty(const OrderItem* item) const;
    double totalWeight(InventoryManager& inventory);
    double heaviestUnitWeight(InventoryManager& inventory);


    bool isFullyDelivered() const;
    bool hasAnyDelivery() const;

  
    void displayOrder() const;
};