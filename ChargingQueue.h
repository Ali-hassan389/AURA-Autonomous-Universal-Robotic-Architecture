#pragma once
#include "Common.h"

class ChargingQueueNode {
public:
    string robotId;
    ChargingQueueNode* next;
    explicit ChargingQueueNode(const string& id);
};

class ChargingQueue {
private:
    ChargingQueueNode* front;
    ChargingQueueNode* rear;
    int size;

public:
    ChargingQueue();
    ~ChargingQueue();
    ChargingQueue(const ChargingQueue&) = delete;
    ChargingQueue& operator=(const ChargingQueue&) = delete;

    void enqueue(const string& robotId);
    string dequeue();
    string peekFront() const;
    bool removeRobot(const string& robotId);
    bool contains(const string& robotId) const;
    bool isEmpty() const;
    int getSize() const;
    void display() const;
};
