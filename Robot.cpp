#include "Robot.h"
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

Robot::Robot(const string& id, const string& location, double batteryLevel, double capacity)
    : robotId(toUpperCase(trimText(id))), currentLocationId(toUpperCase(trimText(location))),
      battery(batteryLevel), maxCapacity(capacity), status(Robot_Available), assignedTask("") {}

Robot::~Robot() {}

string Robot::getId() const { return robotId; }
string Robot::getLocation() const { return currentLocationId; }
double Robot::getBattery() const { return battery; }
double Robot::getCapacity() const { return maxCapacity; }
RobotStatus Robot::getStatus() const { return status; }
string Robot::getTask() const { return assignedTask; }
void Robot::setLocation(const string& location) { currentLocationId = toUpperCase(trimText(location)); }
bool Robot::setBattery(double level) {
    if (!isfinite(level) || level < 0.0 || level > Full_Battery) return false;
    battery = level;
    return true;
}
void Robot::setStatus(RobotStatus newStatus) { status = newStatus; }
void Robot::setTask(const string& task) { assignedTask = trimText(task); }

bool Robot::canCarryUnit(double unitWeight) const {
    return unitWeight > 0.0 && unitWeight <= maxCapacity;
}

double Robot::consumption(int distance, double carriedWeight) const {
    if (distance < 0 || !isfinite(carriedWeight) || carriedWeight < 0.0) return -1.0;
    return legConsumption(distance, getBatteryRate(), carriedWeight, maxCapacity);
}

bool Robot::needsCharging() const { return battery < Low_Battery_Limit; }
bool Robot::canStartPlan(double estimatedConsumption) const {
    return estimatedConsumption >= 0.0 && battery >= estimatedConsumption + Safety_Reserve;
}

void Robot::display() const {
    cout << left << setw(9) << robotId << setw(10) << getTypeName() << setw(10) << currentLocationId
         << right << setw(8) << fixed << setprecision(1) << battery
         << setw(10) << setprecision(1) << maxCapacity << "  "
         << left << setw(22) << robotStatusToText(status) << setw(12) << assignedTask;
    if (needsCharging()) cout << " NEEDS CHARGING";
    cout << '\n';
}

Picker::Picker(const string& id, const string& typeText, const string& location, double batteryLevel, double capacity)
    : Robot(id, location, batteryLevel, capacity) { (void)typeText; }
double Picker::getBatteryRate() const { return Rate_Picker; }
RobotType Picker::getType() const { return Type_Picker; }
string Picker::getTypeName() const { return "PICKER"; }
bool Picker::isCapacityValid() const { return maxCapacity > 0.0 && maxCapacity <= Picker_MaxCapacity; }
bool Picker::canCarryUnit(double unitWeight) const {
    return Robot::canCarryUnit(unitWeight) && unitWeight <= Picker_MaxUnitWeight;
}
bool Picker::eligibleForOrder(double orderWeight, double heaviestUnit) const {
    (void)orderWeight;
    return heaviestUnit <= Picker_MaxUnitWeight && canCarryUnit(heaviestUnit);
}

Carrier::Carrier(const string& id, const string& typeText, const string& location, double batteryLevel, double capacity)
    : Robot(id, location, batteryLevel, capacity) { (void)typeText; }
double Carrier::getBatteryRate() const { return Rate_Carrier; }
RobotType Carrier::getType() const { return Type_Carrier; }
string Carrier::getTypeName() const { return "CARRIER"; }
bool Carrier::isCapacityValid() const {
    return maxCapacity >= Carrier_MinCapacity && maxCapacity <= Carrier_MaxCapacity;
}
bool Carrier::eligibleForOrder(double orderWeight, double heaviestUnit) const {
    (void)orderWeight;
    return canCarryUnit(heaviestUnit);
}

HeavyDuty::HeavyDuty(const string& id, const string& typeText, const string& location, double batteryLevel, double capacity)
    : Robot(id, location, batteryLevel, capacity) { (void)typeText; }
double HeavyDuty::getBatteryRate() const { return Rate_Heavy; }
RobotType HeavyDuty::getType() const { return Type_Heavy; }
string HeavyDuty::getTypeName() const { return "HEAVY"; }
bool HeavyDuty::isCapacityValid() const { return maxCapacity > 0.0 && maxCapacity <= Heavy_MaxCapacity; }
bool HeavyDuty::eligibleForOrder(double orderWeight, double heaviestUnit) const {
    return (orderWeight > Heavy_MinOrderWeight || heaviestUnit > Heavy_MinUnitWeight) && canCarryUnit(heaviestUnit);
}
