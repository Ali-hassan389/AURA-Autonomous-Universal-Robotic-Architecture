#pragma once

#include "common.h"

class InventoryManager;

class OrderItem {
public:
    string productId;
    int quantity;       
    int deliveredQty;   
    OrderItem* next;    

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

    bool addItem(string productId, int qty);
    OrderItem* findItem(string productId);

    int remainingQty(const OrderItem* item) const;
    double totalWeight(InventoryManager& inventory);
    double heaviestUnitWeight(InventoryManager& inventory);

    bool isFullyDelivered() const;
    bool hasAnyDelivery() const;

    void displayOrder() const;
};
