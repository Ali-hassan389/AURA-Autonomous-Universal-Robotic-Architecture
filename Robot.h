#pragma once
#include"Common.h"
class Robot {
protected:
	string robotId;
	string currentLocationId;
	double battery;
	double maxCapacity;
	RobotStatus status;
	string assignedTask;
public:
	// Constructor and virtual destructor
	Robot(const string& id, const string& location,
		double batteryLevel, double capacity);
	virtual ~Robot();
	// Pure virtual functions: make Robot an abstract class
	virtual double getBatteryRate() const = 0;
	virtual RobotType getType() const = 0;
	virtual string getTypeName() const = 0;
	virtual bool isCapacityValid() const = 0;
	virtual bool canCarryUnit(double unitWeight) const;
	virtual bool eligibleForOrder(
		double orderWeight, double heaviestUnit) const = 0;
	// Getters
	string getId() const;
	string getLocation() const;
	double getBattery() const;
	double getCapacity() const;
	RobotStatus getStatus() const;
	string getTask() const;

	// Setters
	void setLocation(const string& location);
	bool setBattery(double level);
	void setStatus(RobotStatus newStatus);
	void setTask(const string& task);
	double consumption(int distance, double carriedWeight) const;
	bool needsCharging() const;
	bool canStartPlan(double estimatedConsumption) const;

	void display() const;
};


// Picker robot
class Picker : public Robot {
public:
	Picker(const string& id, const string& location,
		double batteryLevel, double capacity);

	double getBatteryRate() const override;
	RobotType getType() const override;
	string getTypeName() const override;

	bool isCapacityValid() const override;
	bool canCarryUnit(double unitWeight) const override;

	bool eligibleForOrder(
		double orderWeight, double heaviestUnit) const override;
};


// Carrier robot
class Carrier : public Robot {
public:
	Carrier(const string& id, const string& location,
		double batteryLevel, double capacity);

	double getBatteryRate() const override;
	RobotType getType() const override;
	string getTypeName() const override;

	bool isCapacityValid() const override;

	bool eligibleForOrder(
		double orderWeight, double heaviestUnit) const override;
};


// Heavy-duty robot
class HeavyDuty : public Robot {
public:
	HeavyDuty(const string& id, const string& location,
		double batteryLevel, double capacity);
	double getBatteryRate() const override;
	RobotType getType() const override;
	string getTypeName() const override;
	bool isCapacityValid() const override;
	bool eligibleForOrder(
		double orderWeight, double heaviestUnit) const override;
};