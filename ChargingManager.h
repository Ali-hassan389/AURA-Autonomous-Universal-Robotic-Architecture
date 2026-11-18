#pragma once
#include "ChargingStation.h"

class Graph;
class RobotManager;
class Robot;

class ChargingManager {
private:
    ChargingStation* stationHead;
    int stationCount;
    Graph& graph;
    RobotManager& robots;

    ChargingStation* findStation(const string& stationId) const;
    bool robotIsInAnySlot(const string& robotId) const;
    bool robotIsInAnyQueue(const string& robotId) const;
    void appendStation(ChargingStation* station);

public:
    ChargingManager(Graph& warehouseGraph, RobotManager& robotManager);
    ~ChargingManager();
    ChargingManager(const ChargingManager&) = delete;
    ChargingManager& operator=(const ChargingManager&) = delete;

    bool loadStations(const string& file);
    void displayAllStations();
    void listRobotsNeedingCharging();
    ChargingStation* findStationForRobot(Robot* robot, bool& hasFreeSlot);
    bool isSlotAvailable(const string& stationId);
    bool addToWaitingQueue(Robot* robot, ChargingStation* station);
    bool startCharging(Robot* robot, ChargingStation* station);
    bool requestCharging(const string& robotId);
    bool completeCharging(const string& stationId, const string& robotId);
    bool updateRobotBattery(const string& robotId, double level);
    bool removeFromQueue(const string& robotId);
    bool returnToAvailable(Robot* robot);
    int getStationCount() const;
};

void runChargingMenu(ChargingManager& charging, RobotManager& robots);
