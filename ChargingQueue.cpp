#include "ChargingQueue.h"
#include <iostream>

using namespace std;

ChargingQueueNode::ChargingQueueNode(const string& id)
    : robotId(toUpperCase(trimText(id))), next(nullptr) {}

ChargingQueue::ChargingQueue() : front(nullptr), rear(nullptr), size(0) {}

ChargingQueue::~ChargingQueue() {
    while (front) {
        ChargingQueueNode* old = front;
        front = front->next;
        delete old;
    }
    rear = nullptr;
    size = 0;
}

void ChargingQueue::enqueue(const string& robotId) {
    string id = toUpperCase(trimText(robotId));
    if (id.empty() || contains(id)) return;
    ChargingQueueNode* node = new ChargingQueueNode(id);
    if (!rear) front = rear = node;
    else { rear->next = node; rear = node; }
    ++size;
}

string ChargingQueue::dequeue() {
    if (!front) return "";
    ChargingQueueNode* old = front;
    string id = old->robotId;
    front = front->next;
    if (!front) rear = nullptr;
    delete old;
    --size;
    return id;
}

string ChargingQueue::peekFront() const { return front ? front->robotId : ""; }

bool ChargingQueue::removeRobot(const string& rawId) {
    string id = toUpperCase(trimText(rawId));
    ChargingQueueNode* current = front;
    ChargingQueueNode* previous = nullptr;
    while (current) {
        if (current->robotId == id) {
            if (previous) previous->next = current->next;
            else front = current->next;
            if (rear == current) rear = previous;
            delete current;
            --size;
            return true;
        }
        previous = current;
        current = current->next;
    }
    return false;
}

bool ChargingQueue::contains(const string& rawId) const {
    string id = toUpperCase(trimText(rawId));
    for (ChargingQueueNode* node = front; node; node = node->next)
        if (node->robotId == id) return true;
    return false;
}

bool ChargingQueue::isEmpty() const { return front == nullptr; }
int ChargingQueue::getSize() const { return size; }

void ChargingQueue::display() const {
    cout << "Front";
    for (ChargingQueueNode* node = front; node; node = node->next) cout << " -> " << node->robotId;
    cout << " -> NULL\n";
}
