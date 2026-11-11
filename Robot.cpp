#include"Robot.h"
#include<iostream>
#include<iomanip>
using namespace std;
Robot::Robot(const string& id, const string& location,
	double batteryLevel, double capacity)
{
	robotId = id;
	currentLocationId = location;
	battery = batteryLevel;
	maxCapacity = capacity;
	status = Robot_Available;
	assignedTask = "";
}
Robot::~Robot()
{
}
string Robot::getId() const
{
	return robotId;
}
string Robot::getLocation() const
{
	return currentLocationId;
}
double Robot::getBattery() const
{
	return battery;
}
double Robot::getCapacity() const
{
	return maxCapacity;
}
RobotStatus Robot::getStatus() const
{
	return status;
}
string Robot::getTask() const
{
	return assignedTask;
}
void Robot::setLocation(const string& location)
{
	currentLocationId = location;
}
bool Robot::setBattery(double level)
{
	if (level < 0 || level > 100)
		return false;
	battery = level;
	return true;
}
void Robot::setStatus(RobotStatus newStatus)
{
	status = newStatus;
}
void Robot::setTask(const string& task)
{
	assignedTask = task;
}
bool Robot::canCarryUnit(double unitWeight) const
{
	return unitWeight > 0 && unitWeight <= maxCapacity;
}
double Robot::consumption(int distance, double carriedWeight) const
{
	return legConsumption(
		distance,
		getBatteryRate(),
		carriedWeight,
		maxCapacity
	);
}
bool Robot::needsCharging() const
{
	return battery < 20.0;
}
bool Robot::canStartPlan(double estimatedConsumption) const
{
	return battery >= estimatedConsumption + 10.0;
}
void Robot::display() const
{
	cout << left
		<< setw(10) << robotId
		<< setw(12) << getTypeName()
		<< setw(12) << currentLocationId
		<< right << setw(8) << fixed << setprecision(1) << battery
		<< setw(12) << maxCapacity
		<< "   ";

	switch (status)
	{
	case Robot_Available:
		cout << "Available";
		break;

	case Robot_Assigned:
		cout << "Assigned";
		break;

	case Robot_WaitingforCharging:
		cout << "Waiting";
		break;

	case Robot_Charging:
		cout << "Charging";
		break;

	case Robot_UnAvailable:
		cout << "Unavailable";
		break;

	default:
		cout << "Unknown";
	}

	cout << "   " << assignedTask;

	if (needsCharging())
		cout << "   NEEDS CHARGING";

	cout << endl;
}
Picker::Picker(const string& id, const string& location,
	double batteryLevel, double capacity)
	: Robot(id, location, batteryLevel, capacity)
{
}
double Picker::getBatteryRate() const
{
	return 0.20;
}
RobotType Picker::getType() const
{
	return Type_Picker;
}
string Picker::getTypeName() const
{
	return "PICKER";
}
bool Picker::isCapacityValid() const
{
	return maxCapacity > 0 && maxCapacity <= 15;
}
bool Picker::canCarryUnit(double unitWeight) const
{
	return unitWeight > 0 &&
		unitWeight <= 5 &&
		unitWeight <= maxCapacity;
}
bool Picker::eligibleForOrder(
	double orderWeight, double heaviestUnit) const
{
	return heaviestUnit > 0 &&
		heaviestUnit <= 5 &&
		canCarryUnit(heaviestUnit);
}
Carrier::Carrier(const string& id, const string& location,
	double batteryLevel, double capacity)
	: Robot(id, location, batteryLevel, capacity)
{
}
double Carrier::getBatteryRate() const
{
	return 0.25;
}
RobotType Carrier::getType() const
{
	return Type_Carrier;
}
string Carrier::getTypeName() const
{
	return "CARRIER";
}
bool Carrier::isCapacityValid() const
{
	return maxCapacity >= 15 && maxCapacity <= 30;
}
bool Carrier::eligibleForOrder(
	double orderWeight, double heaviestUnit) const
{
	return canCarryUnit(heaviestUnit);
}
HeavyDuty::HeavyDuty(const string& id, const string& location,
	double batteryLevel, double capacity)
	: Robot(id, location, batteryLevel, capacity)
{
}
double HeavyDuty::getBatteryRate() const
{
	return 0.35;
}
RobotType HeavyDuty::getType() const
{
	return Type_Heavy;
}
string HeavyDuty::getTypeName() const
{
	return "HEAVY";
}
bool HeavyDuty::isCapacityValid() const
{
	return maxCapacity > 0 && maxCapacity <= 60;
}
bool HeavyDuty::eligibleForOrder(
	double orderWeight, double heaviestUnit) const
{
	return (orderWeight > 15 || heaviestUnit > 10) &&
		canCarryUnit(heaviestUnit);
}