#include <climits>
#include "ChargingManager.h"
#include "Graph.h"
#include "RobotManager.h"
#include "Robot.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

ChargingManager::ChargingManager(Graph& warehouseGraph, RobotManager& robotManager)
    : stationHead(nullptr), stationCount(0), graph(warehouseGraph), robots(robotManager) {}

ChargingManager::~ChargingManager() {
    while (stationHead) {
        ChargingStation* old = stationHead;
        stationHead = stationHead->getNext();
        delete old;
    }
    stationCount = 0;
}

void ChargingManager::appendStation(ChargingStation* station) {
    if (!stationHead) stationHead = station;
    else {
        ChargingStation* tail = stationHead;
        while (tail->getNext()) tail = tail->getNext();
        tail->setNext(station);
    }
    ++stationCount;
}

ChargingStation* ChargingManager::findStation(const string& rawId) const {
    string id = toUpperCase(trimText(rawId));
    for (ChargingStation* station = stationHead; station; station = station->getNext())
        if (station->getId() == id) return station;
    return nullptr;
}

bool ChargingManager::robotIsInAnySlot(const string& id) const {
    for (ChargingStation* station = stationHead; station; station = station->getNext())
        if (station->isCharging(id)) return true;
    return false;
}

bool ChargingManager::robotIsInAnyQueue(const string& id) const {
    for (ChargingStation* station = stationHead; station; station = station->getNext())
        if (station->isQueued(id)) return true;
    return false;
}

bool ChargingManager::loadStations(const string& file) {
    ifstream input(file);
    if (!input.is_open()) { cout << "Could not open chargers file: " << file << '\n'; return false; }
    string line; int lineNumber = 0;
    while (getline(input, line)) {
        ++lineNumber; line = trimText(line);
        if (line.empty() || line[0] == '#') continue;
        string fields[8]; int fieldCount = splitLine(line, ',', fields, 8);
        int slots = 0; bool validNumber = false;
        if (fieldCount == 3) {
            try { size_t used = 0; slots = stoi(fields[2], &used); validNumber = used == fields[2].length(); }
            catch (...) { validNumber = false; }
        }
        string stationId = toUpperCase(fields[0]);
        string locationId = toUpperCase(fields[1]);
        if (fieldCount != 3 || stationId.empty() || findStation(stationId) || !validNumber ||
            slots < 1 || slots > MaxSlots_PerStation || !graph.locationExists(locationId) ||
            graph.getLocationType(locationId) != Location_Charging) {
            cout << "Warning: invalid chargers.txt line " << lineNumber << ".\n";
            continue;
        }
        appendStation(new ChargingStation(stationId, locationId, slots));
    }
    return true;
}

void ChargingManager::displayAllStations() {
    if (!stationHead) { cout << "No charging stations loaded.\n"; return; }
    for (ChargingStation* station = stationHead; station; station = station->getNext()) {
        cout << "\nStation " << station->getId() << " | Location " << station->getLocation()
             << " | Slots: " << station->getTotalSlots() << " | Free: " << station->freeSlots() << '\n';
        cout << "Charging robots: ";
        bool any = false;
        for (RobotNode* node = robots.getHead(); node; node = node->next) {
            if (station->isCharging(node->robot->getId())) { cout << node->robot->getId() << ' '; any = true; }
        }
        if (!any) cout << "None";
        cout << "\nWaiting queue: "; station->getWaitingQueue().display();
    }
}

void ChargingManager::listRobotsNeedingCharging() {
    bool any = false;
    cout << "Robots below " << Low_Battery_Limit << "% battery (Available / Waiting):\n";
    for (RobotNode* node = robots.getHead(); node; node = node->next) {
        Robot* robot = node->robot;
        if (robot && robot->needsCharging() &&
            (robot->getStatus() == Robot_Available || robot->getStatus() == Robot_WaitingforCharging)) {
            robot->display(); any = true;
        }
    }
    if (!any) cout << "None.\n";
}

ChargingStation* ChargingManager::findStationForRobot(Robot* robot, bool& hasFreeSlot) {
    hasFreeSlot = false;
    if (!robot) return nullptr;
    ChargingStation* nearestFree = nullptr;
    ChargingStation* nearestAny = nullptr;
    int freeDistance = INT_MAX, anyDistance = INT_MAX;
    for (ChargingStation* station = stationHead; station; station = station->getNext()) {
        int distance = graph.shortestDistance(robot->getLocation(), station->getLocation());
        if (distance < 0) continue;
        double requiredBattery = robot->consumption(distance, 0.0);
        if (requiredBattery < 0.0 || robot->getBattery() < requiredBattery) continue;
        if (distance < anyDistance || (distance == anyDistance && (!nearestAny || station->getId() < nearestAny->getId()))) {
            anyDistance = distance; nearestAny = station;
        }
        if (station->hasFreeSlot() &&
            (distance < freeDistance || (distance == freeDistance && (!nearestFree || station->getId() < nearestFree->getId())))) {
            freeDistance = distance; nearestFree = station;
        }
    }
    if (nearestFree) { hasFreeSlot = true; return nearestFree; }
    return nearestAny;
}

bool ChargingManager::isSlotAvailable(const string& stationId) {
    ChargingStation* station = findStation(stationId);
    return station && station->hasFreeSlot();
}

bool ChargingManager::addToWaitingQueue(Robot* robot, ChargingStation* station) {
    if (!robot || !station || robotIsInAnySlot(robot->getId()) || robotIsInAnyQueue(robot->getId()) ||
        (robot->getStatus() != Robot_Available && robot->getStatus() != Robot_WaitingforCharging)) return false;
    if (station->hasFreeSlot()) return false;
    station->getWaitingQueue().enqueue(robot->getId());
    if (!station->getWaitingQueue().contains(robot->getId())) return false;
    robot->setStatus(Robot_WaitingforCharging);
    return true;
}

bool ChargingManager::startCharging(Robot* robot, ChargingStation* station) {
    if (!robot || !station || robotIsInAnySlot(robot->getId()) ||
        (robot->getStatus() != Robot_Available && robot->getStatus() != Robot_WaitingforCharging)) return false;
    if (robotIsInAnyQueue(robot->getId())) return false;
    if (!station->occupySlot(robot->getId())) return false;
    robot->setStatus(Robot_Charging);
    robot->setLocation(station->getLocation());
    return true;
}

bool ChargingManager::requestCharging(const string& rawId) {
    string id = toUpperCase(trimText(rawId));
    Robot* robot = robots.findRobot(id);
    if (!robot) { cout << "Robot not found.\n"; return false; }
    if (robot->getStatus() == Robot_Assigned || !robot->getTask().empty()) {
        cout << "An assigned robot cannot be sent for charging.\n"; return false;
    }
    if (robot->getStatus() == Robot_Charging || robotIsInAnySlot(id)) {
        cout << "Robot is already charging.\n"; return false;
    }
    if (robot->getBattery() >= Low_Battery_Limit) {
        cout << "Robot is not eligible: battery is not below " << Low_Battery_Limit << "%.\n"; return false;
    }
    if (robot->getStatus() != Robot_Available && robot->getStatus() != Robot_WaitingforCharging) {
        cout << "Robot is not Available or Waiting for Charging.\n"; return false;
    }
    if (robotIsInAnyQueue(id)) { cout << "Robot is already in a charging queue.\n"; return false; }
    bool hasFreeSlot = false;
    ChargingStation* station = findStationForRobot(robot, hasFreeSlot);
    if (!station) {
        cout << "cannot reach any station, handle manually\n";
        if (readYesNo("Set robot Unavailable?")) robot->setStatus(Robot_UnAvailable);
        return false;
    }
    if (hasFreeSlot) {
        if (startCharging(robot, station)) {
            cout << "Robot " << id << " started charging at " << station->getId() << ".\n";
            return true;
        }
        return false;
    }
    if (addToWaitingQueue(robot, station)) {
        cout << "All reachable stations are full. Robot added to " << station->getId() << " queue.\n";
        return true;
    }
    cout << "Could not add robot to the charging queue.\n";
    return false;
}

bool ChargingManager::completeCharging(const string& rawStationId, const string& rawRobotId) {
    ChargingStation* station = findStation(rawStationId);
    string robotId = toUpperCase(trimText(rawRobotId));
    if (!station) { cout << "Charging station not found.\n"; return false; }
    if (!station->isCharging(robotId)) { cout << "Robot is not in a slot at this station.\n"; return false; }
    Robot* robot = robots.findRobot(robotId);
    if (!robot) { cout << "Robot record not found.\n"; return false; }
    robot->setBattery(Full_Battery);
    station->freeSlot(robotId);
    robot->setLocation(station->getLocation());
    robot->setStatus(Robot_Available);
    cout << "Robot " << robotId << " is fully charged and Available.\n";

    while (!station->getWaitingQueue().isEmpty()) {
        string nextId = station->getWaitingQueue().dequeue();
        Robot* nextRobot = robots.findRobot(nextId);
        if (nextRobot && nextRobot->getStatus() == Robot_WaitingforCharging && startCharging(nextRobot, station)) {
            cout << "Robot " << nextId << " automatically started charging.\n";
            break;
        }
        if (nextRobot && nextRobot->getStatus() == Robot_WaitingforCharging) nextRobot->setStatus(Robot_Available);
        cout << "Skipped invalid waiting entry: " << nextId << ".\n";
    }
    cout << "Updated waiting queue: "; station->getWaitingQueue().display();
    return true;
}

bool ChargingManager::updateRobotBattery(const string& robotId, double level) {
    return robots.updateBattery(robotId, level);
}

bool ChargingManager::removeFromQueue(const string& rawId) {
    string id = toUpperCase(trimText(rawId));
    for (ChargingStation* station = stationHead; station; station = station->getNext()) {
        if (station->getWaitingQueue().removeRobot(id)) {
            Robot* robot = robots.findRobot(id);
            if (robot) robot->setStatus(Robot_Available);
            cout << "Robot removed from queue and set to Available.\n";
            return true;
        }
    }
    return false;
}

bool ChargingManager::returnToAvailable(Robot* robot) {
    if (!robot || robot->getStatus() == Robot_Assigned || robot->getStatus() == Robot_Charging ||
        robot->getStatus() == Robot_WaitingforCharging) return false;
    robot->setStatus(Robot_Available);
    return true;
}

int ChargingManager::getStationCount() const { return stationCount; }

void runChargingMenu(ChargingManager& charging, RobotManager& robots) {
    int choice;
    do {
        cout << "\n========== BATTERY AND CHARGING ==========\n"
             << "1. Display all charging stations\n2. List robots needing charging\n3. Request charging\n"
             << "4. Complete charging\n5. Update robot battery\n6. Remove robot from waiting queue\n"
             << "7. Return eligible robot to Available\n0. Back\n";
        choice = readIntInRange("Enter choice: ", 0, 7);
        if (choice == 1) charging.displayAllStations();
        else if (choice == 2) charging.listRobotsNeedingCharging();
        else if (choice == 3) {
            string id = toUpperCase(readNonEmptyString("Robot ID: "));
            charging.requestCharging(id);
        }
        else if (choice == 4) {
            string stationId = toUpperCase(readNonEmptyString("Station ID: "));
            string robotId = toUpperCase(readNonEmptyString("Robot ID in that station: "));
            charging.completeCharging(stationId, robotId);
        }
        else if (choice == 5) {
            string id = toUpperCase(readNonEmptyString("Robot ID: "));
            double level = readDoubleInRange("Battery level (0-100): ", 0.0, 100.0);
            cout << (charging.updateRobotBattery(id, level) ? "Battery updated.\n" : "Robot not found.\n");
        }
        else if (choice == 6) {
            string id = toUpperCase(readNonEmptyString("Robot ID: "));
            cout << (charging.removeFromQueue(id) ? "Queue entry removed.\n" : "Robot was not in a queue.\n");
        }
        else if (choice == 7) {
            string id = toUpperCase(readNonEmptyString("Robot ID: "));
            Robot* robot = robots.findRobot(id);
            cout << (charging.returnToAvailable(robot) ? "Robot is now Available.\n" : "Could not change this robot to Available.\n");
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}
