#include "OrderManager.h"
#include "RobotManager.h"
#include <fstream>

// Retain the supplied inventory reference and initialize the order list and count.
OrderManager::OrderManager(InventoryManager& inventory) : inventory(inventory) {
    head = nullptr;
    tail = nullptr;
    orderCount = 0;
}
// Deletes every order. The queue only has pointers, so it deletes nothing.
OrderManager::~OrderManager() {
    Order* current = head;
    while (current != nullptr) {
        Order* nextOrder = current->next;
        delete current;
        current = nextOrder;
    }
    head = nullptr;
    tail = nullptr;
}
// Operation 1: reads lines like OrderID,CustomerID,ProductID,Qty
// Load comma-separated order lines; invalid rows are reported and skipped.
bool OrderManager::loadOrders(string file) {
    ifstream in(file);
    if (!in.is_open()) {
        cout << "Could not open " << file << endl;
        return false;
    }

    string line;
    int lineNumber = 0;
    while (getline(in, line)) {
        lineNumber++;
        line = trimText(line);
        if (line == "" || line[0] == '#') {
            continue;
        }

        string fields[Max_Fields];
        int count = splitLine(line, ',', fields, Max_Fields);
        if (count != 4) {
            cout << "Warning line " << lineNumber << ": wrong number of fields." << endl;
            continue;
        }

        string cleanOrderId = toUpperCase(trimText(fields[0]));
        string cleanCustomerId = toUpperCase(trimText(fields[1]));
        string cleanProductId = toUpperCase(trimText(fields[2]));

        if (cleanOrderId.empty() || cleanCustomerId.empty() || cleanProductId.empty()) {
            cout << "Warning line " << lineNumber << ": empty identifier." << endl;
            continue;
        }

        // quantity must be only digits and above 0
        bool digitsOnly = (fields[3] != "");
        for (int i = 0; i < (int)fields[3].length(); i++) {
            if (fields[3][i] < '0' || fields[3][i] > '9') {
                digitsOnly = false;
            }
        }
        if (!digitsOnly || fields[3].length() > 6 || stoi(fields[3]) <= 0) {
            cout << "Warning line " << lineNumber << ": bad quantity." << endl;
            continue;
        }
        int qty = stoi(fields[3]);

        if (!inventory.productExists(cleanProductId)) {
            cout << "Warning line " << lineNumber << ": unknown product " << cleanProductId << endl;
            continue;
        }

        // create the order the first time we see its ID
        Order* order = findOrder(cleanOrderId);
        if (order == nullptr) {
            addOrder(cleanOrderId, cleanCustomerId);
            order = findOrder(cleanOrderId);
        }
        if (order == nullptr || order->status != Order_Pending) {
            continue;
        }

        order->addItem(cleanProductId, qty);

        // queue it once it has its first item, so queue order = file order
        if (!order->inQueue) {
            pendingQueue.enqueue(order);
        }
    }
    in.close();
    return true;
}

// Operation 2: new empty Pending order, not in the queue yet
// Create an empty Pending order and append it to the manager-owned list.
bool OrderManager::addOrder(string orderId, string customerId) {
    string cleanOrderId = toUpperCase(trimText(orderId));
    string cleanCustomerId = toUpperCase(trimText(customerId));

    if (cleanOrderId == "" || cleanCustomerId == "") {
        cout << "Order ID and customer ID cannot be empty." << endl;
        return false;
    }
    if (findOrder(cleanOrderId) != nullptr) {
        cout << "Order " << cleanOrderId << " already exists." << endl;
        return false;
    }

    Order* order = new Order(cleanOrderId, cleanCustomerId);
    if (head == nullptr) {
        head = order;
        tail = order;
    }
    else {
        tail->next = order;
        tail = order;
    }
    orderCount++;
    return true;
}

// Operation 3
// Validate order state, product existence, and quantity before adding a line.
bool OrderManager::addItemToOrder(string orderId, string productId, int qty) {
    string cleanOrderId = toUpperCase(trimText(orderId));
    string cleanProductId = toUpperCase(trimText(productId));
    Order* order = findOrder(cleanOrderId);
    if (order == nullptr) {
        cout << "Order " << cleanOrderId << " does not exist." << endl;
        return false;
    }
    if (order->status != Order_Pending) {
        cout << "Items can only be added to a Pending order." << endl;
        return false;
    }
    if (!inventory.productExists(cleanProductId)) {
        cout << "Product " << cleanProductId << " does not exist." << endl;
        return false;
    }
    if (qty <= 0) {
        cout << "Quantity must be above 0." << endl;
        return false;
    }
    return order->addItem(cleanProductId, qty);
}

// Operation 4
// Search the main order list by normalized ID.
Order* OrderManager::findOrder(string orderId) {
    string cleanOrderId = toUpperCase(trimText(orderId));
    Order* current = head;
    while (current != nullptr) {
        if (current->orderId == cleanOrderId) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

// Operation 5: queues every Pending order that has items and is not queued yet
// Queue eligible orders and return the number newly enqueued.
int OrderManager::enqueuePendingOrders() {
    int added = 0;
    Order* current = head;
    while (current != nullptr) {
        if (current->status == Order_Pending && !current->inQueue && current->itemHead != nullptr) {
            pendingQueue.enqueue(current);
            added++;
        }
        current = current->next;
    }
    return added;
}
// Operation 6
// Delegate display of queued orders to the queue object.
void OrderManager::displayPendingQueue() {
    pendingQueue.display();
}
// Operation 7
// Display the front order and queue size, or report an empty queue.
void OrderManager::peekFront() {
    Order* front = pendingQueue.peek();
    if (front == nullptr) {
        cout << "Pending queue is empty." << endl;
        return;
    }
    front->displayOrder();
    cout << "Queue size: " << pendingQueue.getSize() << endl;
}
// Operation 8
// Cancel an eligible order, freeing associated resources where applicable.
bool OrderManager::cancelOrder(string orderId, InventoryManager& inventory, RobotManager& robots) {
    Order* order = findOrder(orderId);
    if (order == nullptr) {
        cout << "Order " << orderId << " does not exist." << endl;
        return false;
    }
    if (order->status == Order_PartiallyFulfilled || order->status == Order_Completed
        || order->status == Order_Cancelled) {
        cout << "Order " << orderId << " is " << orderStatusToText(order->status)
            << " and cannot be cancelled." << endl;
        return false;
    }

    if (order->status == Order_Pending) {
        if (order->inQueue) {
            pendingQueue.removeById(orderId);
        }
    }
    else {
        // Reserved or Assigned: free the stock and the robot
        inventory.releaseAllReservations(orderId);
        if (order->assignedRobotId != "") {
            robots.releaseRobot(order->assignedRobotId);
            order->assignedRobotId = "";
        }
    }

    // the record stays in the main list
    order->status = Order_Cancelled;
    cout << "Order " << orderId << " cancelled." << endl;
    return true;
}

// Operation 9
// Remove cancelled entries from the pending queue and return the removal count.
int OrderManager::removeCancelledFromQueue() {
    return pendingQueue.removeCancelled();
}

// Operation 10: only legal moves are allowed
// Allow only the status transitions encoded below; reject other transitions.
bool OrderManager::updateStatus(string orderId, OrderStatus newStatus) {
    Order* order = findOrder(orderId);
    if (order == nullptr) {
        cout << "Order " << orderId << " does not exist." << endl;
        return false;
    }

    OrderStatus old = order->status;
    bool legal = false;

    if (old == Order_Pending) {
        legal = (newStatus == Order_Reserved || newStatus == Order_Cancelled);
    }
    else if (old == Order_Reserved) {
        legal = (newStatus == Order_Assigned || newStatus == Order_Pending || newStatus == Order_Cancelled);
    }
    else if (old == Order_Assigned) {
        legal = (newStatus == Order_PartiallyFulfilled || newStatus == Order_Completed
            || newStatus == Order_Pending || newStatus == Order_Cancelled);
    }
    else if (old == Order_PartiallyFulfilled) {
        legal = (newStatus == Order_Completed || newStatus == Order_Pending);
    }

    if (!legal) {
        cout << "Cannot change " << orderStatusToText(old) << " to "
            << orderStatusToText(newStatus) << "." << endl;
        return false;
    }
    order->status = newStatus;
    return true;
}
// Operation 11
// Print completed orders followed by pending orders, with an empty marker per section.
void OrderManager::displayCompletedAndPending() {
    cout << "Completed orders:" << endl;
    Order* current = head;
    bool found = false;
    while (current != nullptr) {
        if (current->status == Order_Completed) {
            current->displayOrder();
            found = true;
        }
        current = current->next;
    }
    if (!found) {
        cout << "    (none)" << endl;
    }

    cout << "Pending orders:" << endl;
    current = head;
    found = false;
    while (current != nullptr) {
        if (current->status == Order_Pending) {
            current->displayOrder();
            found = true;
        }
        current = current->next;
    }
    if (!found) {
        cout << "    (none)" << endl;
    }
}

// Print every order in the main list, or indicate that the list is empty.
void OrderManager::displayAllOrders() {
    if (head == nullptr) {
        cout << "No orders." << endl;
        return;
    }
    Order* current = head;
    while (current != nullptr) {
        current->displayOrder();
        current = current->next;
    }
}

// Expose the first order pointer for traversal by callers.
Order* OrderManager::getHead() {
    return head;
}

// Expose the pending queue by reference.
OrderQueue& OrderManager::getQueue() {
    return pendingQueue;
}

// Menu
// Console menu for loading, editing, querying, and displaying orders.
void runOrderMenu(OrderManager& orders, InventoryManager& inventory, RobotManager& robots) {
    while (true) {
        cout << "\n===== ORDER MENU =====" << endl;
        cout << "1. Load orders from file" << endl;
        cout << "2. Add new order" << endl;
        cout << "3. Add item to pending order" << endl;
        cout << "4. Find order" << endl;
        cout << "5. Enqueue pending orders" << endl;
        cout << "6. Display pending queue" << endl;
        cout << "7. Peek front of queue" << endl;
        cout << "8. Cancel order" << endl;
        cout << "9. Remove cancelled orders from queue" << endl;
        cout << "10. Update order status" << endl;
        cout << "11. Display completed and pending orders" << endl;
        cout << "12. Display all orders" << endl;
        cout << "0. Back" << endl;

        int choice = readIntInRange("Choice: ", 0, 12);

        if (choice == 0) {
            return;
        }
        else if (choice == 1) {
            string file = readNonEmptyString("File name: ");
            orders.loadOrders(file);
        }
        else if (choice == 2) {
            string orderId = readNonEmptyString("Order ID: ");
            string customerId = readNonEmptyString("Customer ID: ");
            if (orders.addOrder(orderId, customerId)) {
                cout << "Order created." << endl;
                while (readYesNo("Add a product to this order?")) {
                    string productId = readNonEmptyString("Product ID: ");
                    int qty = readIntInRange("Quantity: ", 1, 100000);
                    orders.addItemToOrder(orderId, productId, qty);
                }
            }
        }
        else if (choice == 3) {
            string orderId = readNonEmptyString("Order ID: ");
            if (orders.findOrder(orderId) == nullptr) {
                cout << "Order not found." << endl;
            }
            else {
                do {
                    string productId = readNonEmptyString("Product ID: ");
                    int qty = readIntInRange("Quantity: ", 1, 100000);
                    orders.addItemToOrder(orderId, productId, qty);
                } while (readYesNo("Add another product?"));
            }
        }
        else if (choice == 4) {
            string orderId = readNonEmptyString("Order ID: ");
            Order* order = orders.findOrder(orderId);
            if (order == nullptr) {
                cout << "Order not found." << endl;
            }
            else {
                order->displayOrder();
            }
        }
        else if (choice == 5) {
            cout << orders.enqueuePendingOrders() << " order(s) added to the queue." << endl;
        }
        else if (choice == 6) {
            orders.displayPendingQueue();
        }
        else if (choice == 7) {
            orders.peekFront();
        }
        else if (choice == 8) {
            string orderId = readNonEmptyString("Order ID to cancel: ");
            orders.cancelOrder(orderId, inventory, robots);
        }
        else if (choice == 9) {
            cout << orders.removeCancelledFromQueue() << " order(s) removed." << endl;
        }
        else if (choice == 10) {
            string orderId = readNonEmptyString("Order ID: ");
            cout << "1. Pending  2. Reserved  3. Assigned  4. Partially Fulfilled  5. Completed  6. Cancelled" << endl;
            int pick = readIntInRange("New status: ", 1, 6);
            OrderStatus statuses[6] = { Order_Pending, Order_Reserved, Order_Assigned,
                Order_PartiallyFulfilled, Order_Completed, Order_Cancelled };
            if (orders.updateStatus(orderId, statuses[pick - 1])) {
                cout << "Status updated." << endl;
            }
        }
        else if (choice == 11) {
            orders.displayCompletedAndPending();
        }
        else if (choice == 12) {
            orders.displayAllOrders();
        }
        pauseScreen();
    }
}
