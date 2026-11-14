#pragma once

#include "Common.h"

struct PlanStop {
    string rackId;
    string rackLocationId;
    string productId;
    int quantity;
    double unitWeight;
    bool collected;
    PlanStop* next;

    PlanStop(string rack, string location, string product, int qty, double weight);
};

struct RouteLeg {
    string fromId;
    string toId;
    RouteResult route;
    int distance;
    double carriedWeight;
    double battery;
    RouteLeg* next;

    RouteLeg(string from, string to, const RouteResult& routeResult,int legDistance, double weight, double batteryUsed);
};

struct PlanTrip {
    int tripNumber;
    string startLocationId;
    PlanStop* stopHead;
    RouteLeg* legHead;
    string packingStationId;
    double totalWeight;
    int totalDistance;
    double totalBattery;
    bool executed;
    PlanTrip* next;

    PlanTrip(int number, string startLocation);
    ~PlanTrip();

    PlanTrip(const PlanTrip&) = delete;
    PlanTrip& operator=(const PlanTrip&) = delete;

    void addStop(PlanStop* stop);
    void addLeg(RouteLeg* leg);
    void display() const;
};

struct PickingPlan {
    string orderId;
    string robotId;
    PlanTrip* tripHead;
    int tripCount;
    int totalDistance;
    double totalBattery;
    PickingPlan* next;

    PickingPlan(string order, string robot);
    ~PickingPlan();

    PickingPlan(const PickingPlan&) = delete;
    PickingPlan& operator=(const PickingPlan&) = delete;

    void addTrip(PlanTrip* trip);
    void display() const;
};
