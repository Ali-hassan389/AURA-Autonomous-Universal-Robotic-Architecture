#pragma once

#include "Common.h"
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
    // Number of order records currently in the main list.
    int orderCount;
    // Queue of pointers to orders; the manager retains ownership of the records.
    OrderQueue pendingQueue;
    // Inventory service used for product validation and related operations.
    InventoryManager& inventory;

public:
    OrderManager(InventoryManager& inventory);
    ~OrderManager();

    OrderManager(const OrderManager&) = delete;
    OrderManager& operator=(const OrderManager&) = delete;

    // Read order rows from a file; returns false if the file cannot be opened.
    bool loadOrders(string file);
    // Add a new empty order if identifiers are valid and unique.
    bool addOrder(string orderId, string customerId);
    // Add a product quantity to an existing Pending order.
    bool addItemToOrder(string orderId, string productId, int qty);
    // Find an order by ID, or return nullptr.
    Order* findOrder(string orderId);
    // Enqueue eligible pending orders and return the number added.
    int enqueuePendingOrders();
    // Display the current pending queue.
    void displayPendingQueue();
    // Display the queue front, if one exists.
    void peekFront();
    // Attempt cancellation, coordinating queue, inventory, and robot state where applicable.
    bool cancelOrder(string orderId, InventoryManager& inventory, RobotManager& robots);
    // Remove cancelled queue entries and return the number removed.
    int removeCancelledFromQueue();
    // Apply a permitted state transition; false when missing or disallowed.
    bool updateStatus(string orderId, OrderStatus newStatus);
    // Display orders in the completed and pending states.
    void displayCompletedAndPending();
    // Display every order in the main list.
    void displayAllOrders();

    // Return the first order pointer for list traversal.
    Order* getHead();
    // Return a mutable reference to the pending queue.
    OrderQueue& getQueue();
};

// Run the interactive order menu using the provided managers.
void runOrderMenu(OrderManager& orders, InventoryManager& inventory, RobotManager& robots);
