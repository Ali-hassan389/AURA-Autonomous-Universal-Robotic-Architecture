#pragma once
#include "ChargingQueue.h"

class ChargingStation {
private:
    string stationId;
    string locationId;
    int totalSlots;
    string slots[10]; // Common.h allows at most 10 slots per station
    ChargingQueue waitingQueue;
    ChargingStation* next;

public:
    ChargingStation(const string& id, const string& location, int slotCount);
    ChargingStation(const ChargingStation&) = delete;
    ChargingStation& operator=(const ChargingStation&) = delete;

    string getId() const;
    string getLocation() const;
    int getTotalSlots() const;
    int freeSlots() const;
    bool hasFreeSlot() const;
    bool occupySlot(const string& robotId);
    bool freeSlot(const string& robotId);
    bool isCharging(const string& robotId) const;
    bool isQueued(const string& robotId) const;
    ChargingQueue& getWaitingQueue();
    const ChargingQueue& getWaitingQueue() const;
    ChargingStation* getNext() const;
    void setNext(ChargingStation* station);
};
