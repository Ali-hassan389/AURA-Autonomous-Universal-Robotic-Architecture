#pragma once

#include "common.h"
#include "Order.h"
#include "OrderQueue.h"
#include "InventoryManager.h"

class RobotManager;

// Owns every order (main linked list) and the queue of pending orders.
// The queue only points to orders, so the orders are deleted here.
class OrderManager {
private:
    Order* head;                // first order in the main list
    Order* tail;                // last order, so adding is O(1)
    int orderCount;
    OrderQueue pendingQueue;
    InventoryManager& inventory;

public:
    OrderManager(InventoryManager& inventory);
    ~OrderManager();

    OrderManager(const OrderManager&) = delete;
    OrderManager& operator=(const OrderManager&) = delete;

    void loadOrders();
    bool addOrder(string orderId, string customerId);
    bool addItemToOrder(string orderId, string productId, int qty);
    Order* findOrder(string orderId);
    int enqueuePendingOrders();
    void displayPendingQueue();
    void peekFront();
    bool cancelOrder(string orderId, InventoryManager& inventory, RobotManager& robots);
    int removeCancelledFromQueue();
    bool updateStatus(string orderId, OrderStatus newStatus);
    void displayCompletedAndPending();
    void displayAllOrders();

    Order* getHead();
    OrderQueue& getQueue();
};

void runOrderMenu(OrderManager& orders, InventoryManager& inventory, RobotManager& robots);