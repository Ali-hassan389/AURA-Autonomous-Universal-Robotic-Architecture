#include "RobotManager.h"
#include "Graph.h"
#include "UndoManager.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

using namespace std;


// ==================== ROBOT NODE ====================

RobotNode::RobotNode(Robot* robotPtr)
{
	robot = robotPtr;
	next = nullptr;
}


// ==================== ROBOT MANAGER ====================

RobotManager::RobotManager(Graph& warehouseGraph)
	: head(nullptr), count(0), graph(warehouseGraph)
{
}


RobotManager::~RobotManager()
{
	RobotNode* current = head;

	while (current != nullptr)
	{
		RobotNode* nextNode = current->next;

		delete current->robot;
		delete current;

		current = nextNode;
	}

	head = nullptr;
	count = 0;
}


// ==================== APPEND ROBOT ====================

void RobotManager::appendRobot(Robot* robot)
{
	if (robot == nullptr)
		return;

	RobotNode* newNode = new RobotNode(robot);

	if (head == nullptr)
	{
		head = newNode;
	}
	else
	{
		RobotNode* current = head;

		while (current->next != nullptr)
		{
			current = current->next;
		}

		current->next = newNode;
	}

	count++;
}


// ==================== FIND ROBOT ====================

Robot* RobotManager::findRobot(const string& id)
{
	RobotNode* current = head;

	while (current != nullptr)
	{
		if (current->robot->getId() == id)
			return current->robot;

		current = current->next;
	}

	return nullptr;
}


// ==================== REGISTER ROBOT ====================

bool RobotManager::registerRobot(
	const string& id,
	RobotType type,
	const string& location,
	double battery,
	double capacity)
{
	if (id.empty() || location.empty())
	{
		cout << "Rejected: ID or location cannot be empty.\n";
		return false;
	}

	if (findRobot(id) != nullptr)
	{
		cout << "Rejected: duplicate robot ID.\n";
		return false;
	}

	if (!graph.locationExists(location))
	{
		cout << "Rejected: location does not exist.\n";
		return false;
	}

	if (battery < 0 || battery > 100)
	{
		cout << "Rejected: battery must be between 0 and 100.\n";
		return false;
	}

	Robot* newRobot = nullptr;

	switch (type)
	{
	case Type_Picker:
		newRobot = new Picker(id, location, battery, capacity);
		break;

	case Type_Carrier:
		newRobot = new Carrier(id, location, battery, capacity);
		break;

	case Type_Heavy:
		newRobot = new HeavyDuty(id, location, battery, capacity);
		break;

	default:
		cout << "Rejected: invalid robot type.\n";
		return false;
	}

	if (!newRobot->isCapacityValid())
	{
		cout << "Rejected: capacity violates type limit.\n";
		delete newRobot;
		return false;
	}

	appendRobot(newRobot);

	cout << "Robot registered successfully.\n";
	return true;
}


// ==================== LOAD ROBOTS ====================

bool RobotManager::loadRobots(const string& file)
{
	ifstream inputFile(file);

	if (!inputFile.is_open())
	{
		cout << "Error: could not open " << file << endl;
		return false;
	}

	string line;
	int lineNumber = 0;
	int loaded = 0;
	int rejected = 0;

	while (getline(inputFile, line))
	{
		lineNumber++;

		line = trimText(line);

		// Ignore blank lines and comments
		if (line.empty() || line[0] == '#')
			continue;

		string fields[6];

		int fieldCount = splitLine(line, ',', fields, 6);

		if (fieldCount != 5)
		{
			cout << "Warning: invalid field count at line "
				<< lineNumber << ".\n";

			rejected++;
			continue;
		}

		string id = fields[0];
		string typeText = toUpperCase(fields[1]);
		string location = fields[2];

		double battery;
		double capacity;

		try
		{
			size_t batteryPosition;
			size_t capacityPosition;

			battery = stod(fields[3], &batteryPosition);
			capacity = stod(fields[4], &capacityPosition);

			if (batteryPosition != fields[3].length() ||
				capacityPosition != fields[4].length())
			{
				throw invalid_argument("Invalid number");
			}
		}
		catch (...)
		{
			cout << "Warning: invalid battery or capacity at line "
				<< lineNumber << ".\n";

			rejected++;
			continue;
		}

		RobotType type;

		if (typeText == "PICKER")
			type = Type_Picker;
		else if (typeText == "CARRIER")
			type = Type_Carrier;
		else if (typeText == "HEAVY")
			type = Type_Heavy;
		else
		{
			cout << "Warning: unknown robot type at line "
				<< lineNumber << ".\n";

			rejected++;
			continue;
		}

		if (registerRobot(id, type, location, battery, capacity))
			loaded++;
		else
			rejected++;
	}

	inputFile.close();

	cout << "\nRobot loading completed.\n";
	cout << "Loaded: " << loaded << endl;
	cout << "Rejected: " << rejected << endl;

	return true;
}


// ==================== DISPLAY ROBOTS ====================

void RobotManager::displayAllRobots()
{
	if (head == nullptr)
	{
		cout << "No robots registered.\n";
		return;
	}

	cout << "\n================ ROBOT FLEET ================\n";

	cout << left
		<< setw(10) << "ID"
		<< setw(12) << "TYPE"
		<< setw(12) << "LOCATION"
		<< right << setw(8) << "BATTERY"
		<< setw(12) << "CAPACITY"
		<< "   STATUS / TASK\n";

	cout << string(85, '-') << endl;

	RobotNode* current = head;

	while (current != nullptr)
	{
		current->robot->display();
		current = current->next;
	}

	cout << "\nTotal robots: " << count << endl;
}


void RobotManager::displayAvailableRobots()
{
	RobotNode* current = head;
	bool found = false;

	cout << "\n========== AVAILABLE ROBOTS ==========\n";

	while (current != nullptr)
	{
		if (current->robot->getStatus() == Robot_Available)
		{
			current->robot->display();
			found = true;
		}

		current = current->next;
	}

	if (!found)
		cout << "No available robots.\n";
}


// ==================== UPDATE BATTERY ====================

bool RobotManager::updateBattery(
	const string& id,
	double level)
{
	Robot* robot = findRobot(id);

	if (robot == nullptr)
	{
		cout << "Robot not found.\n";
		return false;
	}

	if (!robot->setBattery(level))
	{
		cout << "Invalid battery level. Enter 0 to 100.\n";
		return false;
	}

	cout << "Battery updated successfully.\n";
	return true;
}


// ==================== ASSIGN ROBOT ====================

bool RobotManager::assignRobot(
	const string& id,
	const string& taskId)
{
	Robot* robot = findRobot(id);

	if (robot == nullptr)
	{
		cout << "Robot not found.\n";
		return false;
	}

	if (taskId.empty())
	{
		cout << "Task ID cannot be empty.\n";
		return false;
	}

	if (robot->getStatus() != Robot_Available ||
		!robot->getTask().empty())
	{
		cout << "Rejected: robot is not available "
			<< "or already has an active assignment.\n";
		return false;
	}

	robot->setStatus(Robot_Assigned);
	robot->setTask(taskId);

	cout << "Robot assigned successfully.\n";
	return true;
}


// ==================== RELEASE ROBOT ====================

bool RobotManager::releaseRobot(const string& id)
{
	Robot* robot = findRobot(id);

	if (robot == nullptr)
	{
		cout << "Robot not found.\n";
		return false;
	}

	if (robot->getStatus() != Robot_Assigned)
	{
		cout << "Robot is not currently assigned.\n";
		return false;
	}

	robot->setStatus(Robot_Available);
	robot->setTask("");

	cout << "Robot released successfully.\n";

	if (robot->needsCharging())
		cout << "Warning: this robot needs charging.\n";

	return true;
}


// ==================== MAINTENANCE ====================

bool RobotManager::setMaintenance(
	const string& id,
	bool toMaintenance,
	RobotStatus& oldStatus)
{
	Robot* robot = findRobot(id);

	if (robot == nullptr)
	{
		cout << "Robot not found.\n";
		return false;
	}

	oldStatus = robot->getStatus();

	if (toMaintenance)
	{
		if (oldStatus == Robot_Assigned ||
			oldStatus == Robot_Charging ||
			oldStatus == Robot_WaitingforCharging)
		{
			cout << "Rejected: robot is assigned or involved in charging.\n";
			return false;
		}

		if (oldStatus == Robot_UnAvailable)
		{
			cout << "Robot is already unavailable.\n";
			return false;
		}

		if (!robot->getTask().empty())
		{
			cout << "Rejected: robot still has an active task.\n";
			return false;
		}

		robot->setStatus(Robot_UnAvailable);

		cout << "Robot sent to maintenance.\n";
	}
	else
	{
		if (oldStatus != Robot_UnAvailable)
		{
			cout << "Robot is not in maintenance.\n";
			return false;
		}

		robot->setStatus(Robot_Available);

		cout << "Robot returned from maintenance.\n";
	}

	return true;
}


// ==================== RESTORE STATUS ====================

bool RobotManager::restoreStatus(
	const string& id,
	RobotStatus status)
{
	Robot* robot = findRobot(id);

	if (robot == nullptr)
		return false;

	robot->setStatus(status);
	return true;
}


// ==================== ACCESSORS ====================

RobotNode* RobotManager::getHead() const
{
	return head;
}


int RobotManager::getCount() const
{
	return count;
}


// ==================== ROBOT MENU ====================

void runRobotMenu(
	RobotManager& robots,
	Graph& graph,
	UndoManager& undo)
{
	int choice;

	do
	{
		cout << "\n========== ROBOT FLEET MANAGEMENT ==========\n";
		cout << "1. Display all robots\n";
		cout << "2. Display available robots\n";
		cout << "3. Register a robot\n";
		cout << "4. Update robot battery\n";
		cout << "5. Assign robot to task\n";
		cout << "6. Release robot\n";
		cout << "7. Send robot to / return from maintenance\n";
		cout << "0. Back\n";

		choice = readIntInRange("Enter choice: ", 0, 7);

		switch (choice)
		{
		case 1:
			robots.displayAllRobots();
			break;

		case 2:
			robots.displayAvailableRobots();
			break;

		case 3:
		{
			string id = readNonEmptyString("Robot ID: ");

			cout << "1. Picker\n";
			cout << "2. Carrier\n";
			cout << "3. Heavy Duty\n";

			int typeChoice = readIntInRange("Robot type: ", 1, 3);

			RobotType type;

			if (typeChoice == 1)
				type = Type_Picker;
			else if (typeChoice == 2)
				type = Type_Carrier;
			else
				type = Type_Heavy;

			string location =
				readNonEmptyString("Current location ID: ");

			double battery =
				readDoubleInRange("Battery (0-100): ", 0, 100);

			double capacity =
				readDoubleInRange("Maximum capacity (kg): ", 0.01, 60);

			robots.registerRobot(
				id, type, location, battery, capacity
			);

			break;
		}

		case 4:
		{
			string id = readNonEmptyString("Robot ID: ");

			double level =
				readDoubleInRange("New battery level (0-100): ", 0, 100);

			robots.updateBattery(id, level);
			break;
		}

		case 5:
		{
			string id = readNonEmptyString("Robot ID: ");
			string task = readNonEmptyString("Task ID: ");

			robots.assignRobot(id, task);
			break;
		}

		case 6:
		{
			string id = readNonEmptyString("Robot ID: ");

			robots.releaseRobot(id);
			break;
		}

		case 7:
		{
			string id = readNonEmptyString("Robot ID: ");

			cout << "1. Send to maintenance\n";
			cout << "2. Return from maintenance\n";

			int action =
				readIntInRange("Choose action: ", 1, 2);

			RobotStatus oldStatus;

			bool success = robots.setMaintenance(
				id,
				action == 1,
				oldStatus
			);

			if (success)
				undo.recordMaintenance(id, oldStatus);

			break;
		}
		}

		if (choice != 0)
			pauseScreen();

	} while (choice != 0);
}