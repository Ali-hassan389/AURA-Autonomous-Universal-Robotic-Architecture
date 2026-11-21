#include "RobotManager.h"
#include "Graph.h"
#include "UndoManager.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <cmath>

using namespace std;

RobotNode::RobotNode(Robot* robotPtr) : robot(robotPtr), next(nullptr) {}

RobotManager::RobotManager(Graph& warehouseGraph) : head(nullptr), count(0), graph(warehouseGraph) {}

RobotManager::~RobotManager() {
    while (head) {
        RobotNode* old = head;
        head = head->next;
        delete old->robot;
        delete old;
    }
    count = 0;
}

void RobotManager::appendRobot(Robot* robot) {
    RobotNode* node = new RobotNode(robot);
    if (!head) head = node;
    else { RobotNode* tail = head; while (tail->next) tail = tail->next; tail->next = node; }
    ++count;
}

Robot* RobotManager::findRobot(const string& rawId) {
    string id = toUpperCase(trimText(rawId));
    for (RobotNode* node = head; node; node = node->next)
        if (node->robot && node->robot->getId() == id) return node->robot;
    return nullptr;
}

bool RobotManager::registerRobot(const string& rawId, RobotType type, const string& rawLocation, double battery, double capacity) {
    string id = toUpperCase(trimText(rawId));
    string location = toUpperCase(trimText(rawLocation));
    if (id.empty() || location.empty() || findRobot(id) || !graph.locationExists(location) || !isfinite(battery) || !isfinite(capacity) || battery < 0.0 || battery > Full_Battery) return false;
    string typeText = robotTypeToText(type);
    Robot* robot = nullptr;
    if (type == Type_Picker) robot = new Picker(id, typeText, location, battery, capacity);
    else if (type == Type_Carrier) robot = new Carrier(id, typeText, location, battery, capacity);
    else if (type == Type_Heavy) robot = new HeavyDuty(id, typeText, location, battery, capacity);
    if (!robot) return false;
    if (!robot->isCapacityValid()) { delete robot; cout << "Rejected: capacity violates type limit.\n"; return false; }
    appendRobot(robot);
    return true;
}

bool RobotManager::loadRobots(const string& file) {
    ifstream input(file);
    if (!input.is_open()) { cout << "Could not open robots file: " << file << '\n'; return false; }
    string line; int lineNumber = 0;
    while (getline(input, line)) {
        ++lineNumber; line = trimText(line);
        if (line.empty() || line[0] == '#') continue;
        string fields[8]; int fieldCount = splitLine(line, ',', fields, 8);
        if (fieldCount != 5) { cout << "Warning: invalid robots.txt line " << lineNumber << ".\n"; continue; }
        RobotType type;
        double battery = 0.0, capacity = 0.0;
        bool numbersValid = true;
        try {
            size_t usedBattery = 0, usedCapacity = 0;
            battery = stod(fields[3], &usedBattery); capacity = stod(fields[4], &usedCapacity);
            numbersValid = usedBattery == fields[3].length() && usedCapacity == fields[4].length();
        } catch (const invalid_argument&) { numbersValid = false; }
          catch (const out_of_range&) { numbersValid = false; }
        if (!parseRobotType(fields[1], type) || !numbersValid ||
            !registerRobot(fields[0], type, fields[2], battery, capacity))
            cout << "Warning: rejected robots.txt line " << lineNumber << ".\n";
    }
    return true;
}

void RobotManager::displayAllRobots() {
    if (!head) { cout << "No robots registered.\n"; return; }
    cout << left << setw(9) << "ID" << setw(10) << "TYPE" << setw(10) << "LOCATION"
         << right << setw(8) << "BATTERY" << setw(10) << "CAPACITY" << "  "
         << left << setw(22) << "STATUS" << "TASK\n";
    for (RobotNode* node = head; node; node = node->next) node->robot->display();
}

void RobotManager::displayAvailableRobots() {
    bool found = false;
    for (RobotNode* node = head; node; node = node->next) {
        if (node->robot->getStatus() == Robot_Available) {
            if (!found) cout << "Available robots:\n";
            node->robot->display(); found = true;
        }
    }
    if (!found) cout << "No available robots.\n";
}

bool RobotManager::updateBattery(const string& id, double level) {
    Robot* robot = findRobot(id);
    return robot && robot->setBattery(level);
}

bool RobotManager::assignRobot(const string& id, const string& taskId) {
    Robot* robot = findRobot(id);
    if (!robot || taskId.empty() || robot->getStatus() != Robot_Available || !robot->getTask().empty()) return false;
    robot->setStatus(Robot_Assigned); robot->setTask(taskId);
    return true;
}

bool RobotManager::releaseRobot(const string& id) {
    Robot* robot = findRobot(id);
    if (!robot || robot->getStatus() != Robot_Assigned) return false;
    robot->setStatus(Robot_Available); robot->setTask("");
    if (robot->needsCharging()) cout << "Robot " << robot->getId() << " needs charging.\n";
    return true;
}

bool RobotManager::setMaintenance(const string& id, bool toMaintenance, RobotStatus& oldStatus) {
    Robot* robot = findRobot(id);
    if (!robot) return false;
    oldStatus = robot->getStatus();
    if (toMaintenance) {
        if (oldStatus == Robot_Assigned || oldStatus == Robot_Charging ||
            oldStatus == Robot_WaitingforCharging || oldStatus == Robot_UnAvailable || !robot->getTask().empty()) return false;
        robot->setStatus(Robot_UnAvailable);
        return true;
    }
    if (oldStatus != Robot_UnAvailable) return false;
    robot->setStatus(Robot_Available);
    return true;
}

bool RobotManager::restoreStatus(const string& id, RobotStatus status) {
    Robot* robot = findRobot(id);
    if (!robot) return false;
    robot->setStatus(status);
    if (status != Robot_Assigned) robot->setTask("");
    return true;
}

RobotNode* RobotManager::getHead() const { return head; }
int RobotManager::getCount() const { return count; }

void runRobotMenu(RobotManager& robots, Graph& graph, UndoManager& undo) {
    (void)graph;
    int choice;
    do {
        cout << "\n========== ROBOT FLEET MANAGEMENT ==========\n"
             << "1. Display all robots\n2. Display available robots\n3. Register robot\n"
             << "4. Update battery\n5. Assign robot\n6. Release robot\n7. Put robot in / out of maintenance\n0. Back\n";
        choice = readIntInRange("Enter choice: ", 0, 7);
        if (choice == 1) robots.displayAllRobots();
        else if (choice == 2) robots.displayAvailableRobots();
        else if (choice == 3) {
            string id = toUpperCase(readNonEmptyString("Robot ID: "));
            cout << "1. PICKER  2. CARRIER  3. HEAVY\n";
            int typeChoice = readIntInRange("Robot type: ", 1, 3);
            RobotType type = typeChoice == 1 ? Type_Picker : (typeChoice == 2 ? Type_Carrier : Type_Heavy);
            string location = toUpperCase(readNonEmptyString("Current location ID: "));
            double battery = readDoubleInRange("Battery (0-100): ", 0.0, 100.0);
            double capacity = readDoubleInRange("Capacity in kg: ", 0.01, 60.0);
            cout << (robots.registerRobot(id, type, location, battery, capacity) ? "Robot registered.\n" : "Robot rejected. Check ID, location and type capacity.\n");
        }
        else if (choice == 4) {
            string id = toUpperCase(readNonEmptyString("Robot ID: "));
            double level = readDoubleInRange("New battery (0-100): ", 0.0, 100.0);
            cout << (robots.updateBattery(id, level) ? "Battery updated.\n" : "Robot not found or invalid battery.\n");
        }
        else if (choice == 5) {
            string id = toUpperCase(readNonEmptyString("Robot ID: "));
            string task = toUpperCase(readNonEmptyString("Task / Order ID: "));
            cout << (robots.assignRobot(id, task) ? "Robot assigned.\n" : "Assignment rejected. Robot must be Available and unassigned.\n");
        }
        else if (choice == 6) {
            string id = toUpperCase(readNonEmptyString("Robot ID: "));
            cout << (robots.releaseRobot(id) ? "Robot released.\n" : "Release rejected. Robot must have an active assignment.\n");
        }
        else if (choice == 7) {
            string id = toUpperCase(readNonEmptyString("Robot ID: "));
            Robot* robot = robots.findRobot(id);
            if (!robot) cout << "Robot not found.\n";
            else {
                bool toMaintenance = robot->getStatus() != Robot_UnAvailable;
                RobotStatus oldStatus;
                if (robots.setMaintenance(id, toMaintenance, oldStatus)) {
                    undo.recordMaintenance(id, oldStatus);
                    cout << (toMaintenance ? "Robot placed in maintenance.\n" : "Robot returned to Available.\n");
                } else cout << "Maintenance change rejected: robot may be assigned, charging, queued, or already in the requested state.\n";
            }
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}
