#pragma once

#include "Robot.h"

class Graph;
class UndoManager;

// Each node owns one Robot object
class RobotNode {
public:
	Robot* robot;
	RobotNode* next;

	RobotNode(Robot* robotPtr);
};


class RobotManager {
private:
	RobotNode* head;
	int count;
	Graph& graph;

	void appendRobot(Robot* robot);

public:
	explicit RobotManager(Graph& warehouseGraph);
	~RobotManager();

	// Prevent shallow copying of the linked list
	RobotManager(const RobotManager&) = delete;
	RobotManager& operator=(const RobotManager&) = delete;

	// Robot operations
	bool loadRobots(const string& file);

	bool registerRobot(
		const string& id,
		RobotType type,
		const string& location,
		double battery,
		double capacity
	);

	Robot* findRobot(const string& id);

	void displayAllRobots();
	void displayAvailableRobots();

	bool updateBattery(const string& id, double level);

	bool assignRobot(
		const string& id,
		const string& taskId
	);

	bool releaseRobot(const string& id);

	bool setMaintenance(
		const string& id,
		bool toMaintenance,
		RobotStatus& oldStatus
	);

	bool restoreStatus(
		const string& id,
		RobotStatus status
	);

	// Access for Planner and Charging modules
	RobotNode* getHead() const;

	int getCount() const;
};


// Menu is a free function, not a class member
void runRobotMenu(
	RobotManager& robots,
	Graph& graph,
	UndoManager& undo
);
