#include "ChargingStation.h"

ChargingStation::ChargingStation(const string& id, const string& location, int slotCount)
    : stationId(toUpperCase(trimText(id))), locationId(toUpperCase(trimText(location))),
      totalSlots(slotCount), next(nullptr) {
    for (int i = 0; i < MaxSlots_PerStation; ++i) slots[i] = "";
}

string ChargingStation::getId() const { return stationId; }
string ChargingStation::getLocation() const { return locationId; }
int ChargingStation::getTotalSlots() const { return totalSlots; }

int ChargingStation::freeSlots() const {
    int freeCount = 0;
    for (int i = 0; i < totalSlots; ++i) if (slots[i].empty()) ++freeCount;
    return freeCount;
}

bool ChargingStation::hasFreeSlot() const { return freeSlots() > 0; }

bool ChargingStation::occupySlot(const string& rawId) {
    string id = toUpperCase(trimText(rawId));
    if (id.empty() || isCharging(id) || isQueued(id)) return false;
    for (int i = 0; i < totalSlots; ++i) {
        if (slots[i].empty()) { slots[i] = id; return true; }
    }
    return false;
}

bool ChargingStation::freeSlot(const string& rawId) {
    string id = toUpperCase(trimText(rawId));
    for (int i = 0; i < totalSlots; ++i) {
        if (slots[i] == id) { slots[i] = ""; return true; }
    }
    return false;
}

bool ChargingStation::isCharging(const string& rawId) const {
    string id = toUpperCase(trimText(rawId));
    for (int i = 0; i < totalSlots; ++i) if (slots[i] == id) return true;
    return false;
}

bool ChargingStation::isQueued(const string& id) const { return waitingQueue.contains(id); }
ChargingQueue& ChargingStation::getWaitingQueue() { return waitingQueue; }
const ChargingQueue& ChargingStation::getWaitingQueue() const { return waitingQueue; }
ChargingStation* ChargingStation::getNext() const { return next; }
void ChargingStation::setNext(ChargingStation* station) { next = station; }
