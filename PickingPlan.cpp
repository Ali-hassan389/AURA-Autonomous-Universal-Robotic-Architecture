#include "PickingPlan.h"

PlanStop::PlanStop(string rack, string location, string product,
                   int qty, double weight) {
    rackId = toUpperCase(trimText(rack));
    rackLocationId = toUpperCase(trimText(location));
    productId = toUpperCase(trimText(product));
    quantity = qty;
    unitWeight = weight;
    collected = false;
    next = nullptr;
}

RouteLeg::RouteLeg(string from, string to, const RouteResult& routeResult,
                   int legDistance, double weight, double batteryUsed)
    : route(routeResult) {
    fromId = toUpperCase(trimText(from));
    toId = toUpperCase(trimText(to));
    distance = legDistance;
    carriedWeight = weight;
    battery = batteryUsed;
    next = nullptr;
}

PlanTrip::PlanTrip(int number, string startLocation) {
    tripNumber = number;
    startLocationId = toUpperCase(trimText(startLocation));
    stopHead = nullptr;
    legHead = nullptr;
    packingStationId = "";
    totalWeight = 0.0;
    totalDistance = 0;
    totalBattery = 0.0;
    executed = false;
    next = nullptr;
}

PlanTrip::~PlanTrip() {
    PlanStop* stop = stopHead;

    while (stop != nullptr) {
        PlanStop* nextStop = stop->next;
        delete stop;
        stop = nextStop;
    }

    RouteLeg* leg = legHead;

    while (leg != nullptr) {
        RouteLeg* nextLeg = leg->next;
        delete leg;
        leg = nextLeg;
    }

    stopHead = nullptr;
    legHead = nullptr;
}

void PlanTrip::addStop(PlanStop* stop) {
    if (stop == nullptr) {
        return;
    }

    if (stopHead == nullptr) {
        stopHead = stop;
    }
    else {
        PlanStop* current = stopHead;

        while (current->next != nullptr) {
            current = current->next;
        }

        current->next = stop;
    }
}

void PlanTrip::addLeg(RouteLeg* leg) {
    if (leg == nullptr) {
        return;
    }

    if (legHead == nullptr) {
        legHead = leg;
    }
    else {
        RouteLeg* current = legHead;

        while (current->next != nullptr) {
            current = current->next;
        }

        current->next = leg;
    }

    totalDistance += leg->distance;
    totalBattery += leg->battery;
}

void PlanTrip::display() const {
    cout << "Trip " << tripNumber
         << " | Start: " << startLocationId
         << " | Packing: " << packingStationId
         << " | Weight: " << totalWeight
         << " | Distance: " << totalDistance
         << " | Battery: " << totalBattery
         << (executed ? " | EXECUTED" : " | PENDING") << endl;

    PlanStop* stop = stopHead;

    while (stop != nullptr) {
        cout << "    Stop: Rack " << stop->rackId
             << " (" << stop->rackLocationId << ")"
             << " | Product " << stop->productId
             << " | Quantity " << stop->quantity
             << " | Unit weight " << stop->unitWeight
             << (stop->collected ? " | COLLECTED" : " | WAITING") << endl;
        stop = stop->next;
    }

    RouteLeg* leg = legHead;

    while (leg != nullptr) {
        cout << "    Leg: " << leg->fromId << " -> " << leg->toId
             << " | Distance " << leg->distance
             << " | Load " << leg->carriedWeight
             << " | Battery " << leg->battery << endl;
        leg = leg->next;
    }
}

PickingPlan::PickingPlan(string order, string robot) {
    orderId = toUpperCase(trimText(order));
    robotId = toUpperCase(trimText(robot));
    tripHead = nullptr;
    tripCount = 0;
    totalDistance = 0;
    totalBattery = 0.0;
    next = nullptr;
}

PickingPlan::~PickingPlan() {
    PlanTrip* trip = tripHead;

    while (trip != nullptr) {
        PlanTrip* nextTrip = trip->next;
        delete trip;
        trip = nextTrip;
    }

    tripHead = nullptr;
}

void PickingPlan::addTrip(PlanTrip* trip) {
    if (trip == nullptr) {
        return;
    }

    if (tripHead == nullptr) {
        tripHead = trip;
    }
    else {
        PlanTrip* current = tripHead;

        while (current->next != nullptr) {
            current = current->next;
        }

        current->next = trip;
    }

    tripCount++;
    totalDistance += trip->totalDistance;
    totalBattery += trip->totalBattery;
}

void PickingPlan::display() const {
    cout << "Order: " << orderId
         << " | Robot: " << robotId
         << " | Trips: " << tripCount
         << " | Total distance: " << totalDistance
         << " | Total battery: " << totalBattery << endl;

    PlanTrip* trip = tripHead;

    while (trip != nullptr) {
        trip->display();
        trip = trip->next;
    }
}
