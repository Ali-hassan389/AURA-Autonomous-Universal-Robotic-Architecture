#include "Order.h"
#include "InventoryManager.h"

// Initialize one product line; normalize its identifier and start delivered quantity at zero.
OrderItem::OrderItem(string productId, int quantity) {
    this->productId = toUpperCase(trimText(productId));
    this->quantity = quantity;
    deliveredQty = 0;
    next = nullptr;
}

// Initialize an empty pending order and normalize its order and customer identifiers.
Order::Order(string orderId, string customerId) {
    this->orderId = toUpperCase(trimText(orderId));
    this->customerId = toUpperCase(trimText(customerId));
    itemHead = nullptr;
    status = Order_Pending;
    assignedRobotId = "";
    inQueue = false;
    next = nullptr;
}

// Release the linked list of item records owned by this order.
Order::~Order() {
    OrderItem* current = itemHead;

    while (current != nullptr) {
        OrderItem* nextItem = current->next;
        delete current;
        current = nextItem;
    }

    itemHead = nullptr;
}

// Add a positive quantity, accumulating it into an existing product line when present.
bool Order::addItem(string productId, int qty) {
    string cleanProductId = toUpperCase(trimText(productId));

    if (cleanProductId.empty() || qty <= 0) {
        return false;
    }
    OrderItem* existing = findItem(cleanProductId);

    if (existing != nullptr) {
        existing->quantity += qty;
        return true;
    }

    OrderItem* newItem = new OrderItem(cleanProductId, qty);

    if (itemHead == nullptr) {
        itemHead = newItem;
    }
    else {
        OrderItem* last = itemHead;
        while (last->next != nullptr) {
            last = last->next;
        }
        last->next = newItem;
    }
    return true;
}

// Search item lines by normalized product identifier; return nullptr if absent.
OrderItem* Order::findItem(string productId) {
    string cleanProductId = toUpperCase(trimText(productId));
    OrderItem* current = itemHead;

    while (current != nullptr) {
        if (current->productId == cleanProductId) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

// Return undelivered quantity, clamped to zero; a null item contributes nothing.
int Order::remainingQty(const OrderItem* item) const {
    if (item == nullptr) {
        return 0;
    }
    int remaining = item->quantity - item->deliveredQty;
    return remaining > 0 ? remaining : 0;
}

// Sum remaining quantities multiplied by unit weights obtained from inventory.
double Order::totalWeight(InventoryManager& inventory) {
    double total = 0.0;
    OrderItem* current = itemHead;

    while (current != nullptr) {
        int remaining = remainingQty(current);
        double unitWeight = inventory.getUnitWeight(current->productId);

        if (remaining > 0 && unitWeight >= 0.0) {
            total += remaining * unitWeight;
        }

        current = current->next;
    }

    return total;
}

// Find the largest unit weight among lines with remaining quantity.
double Order::heaviestUnitWeight(InventoryManager& inventory) {
    double heaviest = 0.0;
    OrderItem* current = itemHead;

    while (current != nullptr) {
        if (remainingQty(current) > 0) {
            double unitWeight = inventory.getUnitWeight(current->productId);

            if (unitWeight > heaviest) {
                heaviest = unitWeight;
            }
        }

        current = current->next;
    }

    return heaviest;
}

// An empty order is not considered fully delivered; otherwise check every line.
bool Order::isFullyDelivered() const {
    if (itemHead == nullptr) {
        return false;
    }

    OrderItem* current = itemHead;

    while (current != nullptr) {
        if (current->deliveredQty != current->quantity) {
            return false;
        }

        current = current->next;
    }

    return true;
}

// Report whether any item line has a positive delivered quantity.
bool Order::hasAnyDelivery() const {
    OrderItem* current = itemHead;

    while (current != nullptr) {
        if (current->deliveredQty > 0) {
            return true;
        }

        current = current->next;
    }

    return false;
}

// Print the order summary and each item line to standard output.
void Order::displayOrder() const {
    cout << "Order: " << orderId
         << " | Customer: " << customerId
         << " | Status: " << orderStatusToText(status);

    if (!assignedRobotId.empty()) {
        cout << " | Robot: " << assignedRobotId;
    }

    cout << endl;

    if (itemHead == nullptr) {
        cout << "    (no items)" << endl;
        return;
    }

    OrderItem* current = itemHead;

    while (current != nullptr) {
        cout << "    Product: " << current->productId
             << " | Delivered: " << current->deliveredQty
             << " / Ordered: " << current->quantity << endl;
        current = current->next;
    }
}
