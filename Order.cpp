#include "Order.h"
#include "InventoryManager.h"

OrderItem::OrderItem(string productId, int quantity) {
    this->productId = toUpperCase(trimText(productId));
    this->quantity = quantity;
    deliveredQty = 0;
    next = nullptr;
}

Order::Order(string orderId, string customerId) {
    this->orderId = toUpperCase(trimText(orderId));
    this->customerId = toUpperCase(trimText(customerId));
    itemHead = nullptr;
    status = Order_Pending;
    assignedRobotId = "";
    inQueue = false;
    next = nullptr;
}

Order::~Order() {
    OrderItem* current = itemHead;

    while (current != nullptr) {
        OrderItem* nextItem = current->next;
        delete current;
        current = nextItem;
    }

    itemHead = nullptr;
}

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

int Order::remainingQty(const OrderItem* item) const {
    if (item == nullptr) {
        return 0;
    }
    int remaining = item->quantity - item->deliveredQty;
    return remaining > 0 ? remaining : 0;
}

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
