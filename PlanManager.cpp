#include "PlanManager.h"
#include <iostream>

PlanManager::PlanManager(Graph& graphRef, InventoryManager& inventoryRef,
                         OrderManager& ordersRef, RobotManager& robotsRef)
    : graph(graphRef), inventory(inventoryRef), orders(ordersRef), robots(robotsRef) {
    activeHead = nullptr;
    noAvailableCount = 0;
    eligibilityCount = 0;
    capacityCount = 0;
    batteryCount = 0;
    unreachableCount = 0;
}

PlanManager::~PlanManager() {
    deletePlanList(activeHead);
    activeHead = nullptr;
}

void PlanManager::deleteCollectionItems(CollectionItem* head) {
    while (head != nullptr) {
        CollectionItem* next = head->next;
        delete head;
        head = next;
    }
}

void PlanManager::appendCollectionItem(CollectionItem*& head,
                                       CollectionItem*& tail,
                                       string rackId,
                                       string rackLocationId,
                                       string productId,
                                       int quantity,
                                       double unitWeight) {
    if (quantity <= 0) {
        return;
    }

    CollectionItem* item = new CollectionItem;
    item->rackId = toUpperCase(trimText(rackId));
    item->rackLocationId = toUpperCase(trimText(rackLocationId));
    item->productId = toUpperCase(trimText(productId));
    item->quantity = quantity;
    item->unitWeight = unitWeight;
    item->visited = false;
    item->next = nullptr;

    if (head == nullptr) {
        head = item;
        tail = item;
    }
    else {
        tail->next = item;
        tail = item;
    }
}

bool PlanManager::allocateAllItems(Order* order, Robot* robot,
                                   CollectionItem*& items) {
    items = nullptr;
    CollectionItem* tail = nullptr;

    if (order == nullptr || robot == nullptr) {
        return false;
    }

    OrderItem* orderItem = order->itemHead;

    while (orderItem != nullptr) {
        int required = order->remainingQty(orderItem);

        if (required > 0) {
            int requested = required;
            int available = inventory.totalAvailable(orderItem->productId);

            if (available < requested) {
                requested = available;
            }

            if (requested <= 0) {
                deleteCollectionItems(items);
                items = nullptr;
                return false;
            }

            AllocationResult allocation = inventory.allocate(
                orderItem->productId,
                requested,
                robot->getLocation(),
                graph
            );

            if (allocation.totalAllocated <= 0) {
                deleteCollectionItems(items);
                items = nullptr;
                return false;
            }

            AllocationLine* line = allocation.lineHead;

            while (line != nullptr) {
                string location = inventory.getRackLocation(line->rackId);
                appendCollectionItem(
                    items,
                    tail,
                    line->rackId,
                    location,
                    orderItem->productId,
                    line->quantity,
                    inventory.getUnitWeight(orderItem->productId)
                );
                line = line->next;
            }

            if (!allocation.shortage && allocation.totalAllocated < requested) {
                deleteCollectionItems(items);
                items = nullptr;
                return false;
            }
        }

        orderItem = orderItem->next;
    }

    return items != nullptr;
}

bool PlanManager::buildVisitSequence(CollectionItem* items,
                                     string startLocation,
                                     CollectionItem*& sequence) {
    sequence = nullptr;
    CollectionItem* sequenceTail = nullptr;
    string currentLocation = toUpperCase(trimText(startLocation));

    while (true) {
        CollectionItem* best = nullptr;
        int bestDistance = 2147483647;

        CollectionItem* current = items;
        while (current != nullptr) {
            if (!current->visited && current->quantity > 0) {
                int distance = graph.shortestDistance(
                    currentLocation,
                    current->rackLocationId
                );

                if (distance != -1) {
                    bool better = false;

                    if (best == nullptr || distance < bestDistance) {
                        better = true;
                    }
                    else if (distance == bestDistance &&
                             current->rackId < best->rackId) {
                        better = true;
                    }

                    if (better) {
                        best = current;
                        bestDistance = distance;
                    }
                }
            }

            current = current->next;
        }

        if (best == nullptr) {
            current = items;
            while (current != nullptr) {
                if (!current->visited && current->quantity > 0) {
                    deleteCollectionItems(sequence);
                    sequence = nullptr;
                    return false;
                }
                current = current->next;
            }
            break;
        }

        string selectedRack = best->rackId;
        string selectedLocation = best->rackLocationId;
        current = items;

        while (current != nullptr) {
            if (!current->visited && current->quantity > 0 &&
                current->rackId == selectedRack) {
                CollectionItem* copy = new CollectionItem;
                copy->rackId = current->rackId;
                copy->rackLocationId = current->rackLocationId;
                copy->productId = current->productId;
                copy->quantity = current->quantity;
                copy->unitWeight = current->unitWeight;
                copy->visited = true;
                copy->next = nullptr;

                if (sequence == nullptr) {
                    sequence = copy;
                    sequenceTail = copy;
                }
                else {
                    sequenceTail->next = copy;
                    sequenceTail = copy;
                }

                current->visited = true;
            }
            current = current->next;
        }

        currentLocation = selectedLocation;
    }

    return sequence != nullptr;
}

bool PlanManager::appendLeg(PlanTrip* trip, Robot* robot,
                            string fromId, string toId,
                            double carriedWeight) {
    if (trip == nullptr || robot == nullptr) {
        return false;
    }

    RouteResult route = graph.shortestRoute(fromId, toId);

    if (!route.found) {
        return false;
    }

    RouteLeg* leg = new RouteLeg(
        fromId,
        toId,
        route,
        route.totalDistance,
        carriedWeight,
        robot->consumption(route.totalDistance, carriedWeight)
    );

    trip->addLeg(leg);
    return true;
}

bool PlanManager::closeTrip(PickingPlan* plan, PlanTrip*& trip,
                            Robot* robot, string currentLocation,
                            double carriedWeight) {
    if (plan == nullptr || trip == nullptr || robot == nullptr ||
        trip->stopHead == nullptr) {
        return false;
    }

    string station;
    int stationDistance = -1;

    if (!graph.nearestOfType(
            currentLocation,
            Location_Packing,
            station,
            stationDistance)) {
        return false;
    }

    if (!appendLeg(trip, robot, currentLocation, station, carriedWeight)) {
        return false;
    }

    trip->packingStationId = station;
    plan->addTrip(trip);
    trip = nullptr;
    return true;
}

bool PlanManager::buildTrips(PickingPlan* plan, Robot* robot,
                             CollectionItem* sequence) {
    if (plan == nullptr || robot == nullptr || sequence == nullptr) {
        return false;
    }

    PlanTrip* trip = nullptr;
    string currentLocation = robot->getLocation();
    int nextTripNumber = 1;
    double carriedWeight = 0.0;
    CollectionItem* current = sequence;

    while (current != nullptr) {
        while (current->quantity > 0) {
            if (trip == nullptr) {
                trip = new PlanTrip(nextTripNumber, currentLocation);
                carriedWeight = 0.0;
            }

            if (current->unitWeight <= 0.0 ||
                current->unitWeight > robot->getCapacity()) {
                delete trip;
                return false;
            }

            double remainingCapacity = robot->getCapacity() - carriedWeight;
            int fit = static_cast<int>(remainingCapacity / current->unitWeight);

            if (fit <= 0) {
                if (!closeTrip(plan, trip, robot, currentLocation, carriedWeight)) {
                    delete trip;
                    return false;
                }

                PlanTrip* lastTrip = plan->tripHead;
                while (lastTrip->next != nullptr) {
                    lastTrip = lastTrip->next;
                }

                currentLocation = lastTrip->packingStationId;
                nextTripNumber++;
                carriedWeight = 0.0;
                continue;
            }

            int take = current->quantity < fit ? current->quantity : fit;

            if (!appendLeg(
                    trip,
                    robot,
                    currentLocation,
                    current->rackLocationId,
                    carriedWeight)) {
                delete trip;
                return false;
            }

            PlanStop* stop = new PlanStop(
                current->rackId,
                current->rackLocationId,
                current->productId,
                take,
                current->unitWeight
            );
            trip->addStop(stop);

            carriedWeight += take * current->unitWeight;
            if (carriedWeight > trip->totalWeight) {
                trip->totalWeight = carriedWeight;
            }

            current->quantity -= take;
            currentLocation = current->rackLocationId;

            if (current->quantity > 0) {
                if (!closeTrip(plan, trip, robot, currentLocation, carriedWeight)) {
                    delete trip;
                    return false;
                }

                PlanTrip* lastTrip = plan->tripHead;
                while (lastTrip->next != nullptr) {
                    lastTrip = lastTrip->next;
                }

                currentLocation = lastTrip->packingStationId;
                nextTripNumber++;
                carriedWeight = 0.0;
            }
        }

        current = current->next;
    }

    if (trip != nullptr) {
        if (!closeTrip(plan, trip, robot, currentLocation, carriedWeight)) {
            delete trip;
            return false;
        }
    }

    return plan->tripCount > 0;
}

PickingPlan* PlanManager::buildPlanForRobot(Order* order, Robot* robot,
                                            bool partialAllowed) {
    if (order == nullptr || robot == nullptr) {
        return nullptr;
    }

    CollectionItem* items = nullptr;
    if (!allocateAllItems(order, robot, items)) {
        return nullptr;
    }

    CollectionItem* sequence = nullptr;
    if (!buildVisitSequence(items, robot->getLocation(), sequence)) {
        deleteCollectionItems(items);
        return nullptr;
    }

    PickingPlan* plan = new PickingPlan(order->orderId, robot->getId());

    if (!buildTrips(plan, robot, sequence)) {
        delete plan;
        deleteCollectionItems(items);
        deleteCollectionItems(sequence);
        return nullptr;
    }

    if (!robot->canStartPlan(plan->totalBattery)) {
        delete plan;
        deleteCollectionItems(items);
        deleteCollectionItems(sequence);
        return nullptr;
    }

    (void)partialAllowed;
    deleteCollectionItems(items);
    deleteCollectionItems(sequence);
    return plan;
}

bool PlanManager::checkStock(Order* order) {
    if (order == nullptr) {
        return false;
    }

    OrderItem* item = order->itemHead;

    while (item != nullptr) {
        int remaining = order->remainingQty(item);

        if (remaining > 0 &&
            (!inventory.productExists(item->productId) ||
             inventory.totalAvailable(item->productId) < remaining)) {
            return false;
        }

        item = item->next;
    }

    return true;
}

Order* PlanManager::selectNextOrder() {
    QueueNode* node = orders.getQueue().getFrontNode();

    while (node != nullptr) {
        Order* order = node->order;

        if (checkStock(order)) {
            return order;
        }

        cout << "Order " << order->orderId
             << " cannot currently be fully supplied." << endl;

        if (readYesNo("Allow the available quantity as a partial plan?")) {
            return order;
        }

        node = node->next;
    }

    cout << "No feasible pending order was found." << endl;
    return nullptr;
}

bool PlanManager::isBetterPlan(PickingPlan* candidate,
                               PickingPlan* currentBest) {
    if (candidate == nullptr) {
        return false;
    }

    if (currentBest == nullptr) {
        return true;
    }

    if (candidate->tripCount != currentBest->tripCount) {
        return candidate->tripCount < currentBest->tripCount;
    }

    if (candidate->totalDistance != currentBest->totalDistance) {
        return candidate->totalDistance < currentBest->totalDistance;
    }

    return candidate->robotId < currentBest->robotId;
}

Robot* PlanManager::chooseBestRobot(Order* order, PickingPlan*& bestPlan) {
    bestPlan = nullptr;
    noAvailableCount = 0;
    eligibilityCount = 0;
    capacityCount = 0;
    batteryCount = 0;
    unreachableCount = 0;

    if (order == nullptr) {
        return nullptr;
    }

    double orderWeight = order->totalWeight(inventory);
    double heaviestUnit = order->heaviestUnitWeight(inventory);
    Robot* bestRobot = nullptr;
    RobotNode* node = robots.getHead();

    while (node != nullptr) {
        Robot* robot = node->robot;

        if (robot == nullptr || robot->getStatus() != Robot_Available) {
            noAvailableCount++;
        }
        else if (!robot->eligibleForOrder(orderWeight, heaviestUnit)) {
            eligibilityCount++;
        }
        else if (!robot->canCarryUnit(heaviestUnit)) {
            capacityCount++;
        }
        else {
            PickingPlan* candidate = buildPlanForRobot(order, robot, true);

            if (candidate == nullptr) {
                batteryCount++;
            }
            else if (isBetterPlan(candidate, bestPlan)) {
                delete bestPlan;
                bestPlan = candidate;
                bestRobot = robot;
            }
            else {
                delete candidate;
            }
        }

        node = node->next;
    }

    return bestRobot;
}

void PlanManager::displayPlan(PickingPlan* plan) {
    if (plan == nullptr) {
        cout << "No plan available." << endl;
        return;
    }

    plan->display();
}

bool PlanManager::confirmPlan(Order* order, PickingPlan* plan) {
    if (order == nullptr || plan == nullptr) {
        delete plan;
        return false;
    }

    displayPlan(plan);

    if (!readYesNo("Confirm this plan?")) {
        delete plan;
        return false;
    }

    PlanTrip* trip = plan->tripHead;
    bool reservationFailed = false;

    while (trip != nullptr && !reservationFailed) {
        PlanStop* stop = trip->stopHead;

        while (stop != nullptr) {
            if (!inventory.reserve(
                    order->orderId,
                    stop->productId,
                    stop->rackId,
                    stop->quantity)) {
                reservationFailed = true;
                break;
            }

            stop = stop->next;
        }

        trip = trip->next;
    }

    if (reservationFailed) {
        inventory.releaseAllReservations(order->orderId);
        cout << "Reservation failed; all reservations were rolled back." << endl;
        delete plan;
        return false;
    }

    if (!orders.getQueue().removeById(order->orderId)) {
        inventory.releaseAllReservations(order->orderId);
        delete plan;
        return false;
    }

    if (!orders.updateStatus(order->orderId, Order_Reserved) ||
        !robots.assignRobot(plan->robotId, order->orderId)) {
        inventory.releaseAllReservations(order->orderId);
        orders.updateStatus(order->orderId, Order_Pending);
        orders.getQueue().pushFront(order);
        delete plan;
        return false;
    }

    order->assignedRobotId = plan->robotId;
    orders.updateStatus(order->orderId, Order_Assigned);
    appendActivePlan(plan);
    return true;
}

void PlanManager::appendActivePlan(PickingPlan* plan) {
    if (plan == nullptr) {
        return;
    }

    plan->next = activeHead;
    activeHead = plan;
}

PickingPlan* PlanManager::findPlan(string orderId) {
    string cleanOrderId = toUpperCase(trimText(orderId));
    PickingPlan* current = activeHead;

    while (current != nullptr) {
        if (current->orderId == cleanOrderId) {
            return current;
        }

        current = current->next;
    }

    return nullptr;
}

void PlanManager::discardPlan(string orderId) {
    string cleanOrderId = toUpperCase(trimText(orderId));
    PickingPlan* previous = nullptr;
    PickingPlan* current = activeHead;

    while (current != nullptr) {
        if (current->orderId == cleanOrderId) {
            if (previous == nullptr) {
                activeHead = current->next;
            }
            else {
                previous->next = current->next;
            }

            delete current;
            return;
        }

        previous = current;
        current = current->next;
    }
}

void PlanManager::deletePlanList(PickingPlan* head) {
    while (head != nullptr) {
        PickingPlan* next = head->next;
        delete head;
        head = next;
    }
}

void PlanManager::processNextOrder() {
    Order* order = selectNextOrder();

    if (order == nullptr) {
        return;
    }

    PickingPlan* bestPlan = nullptr;
    Robot* bestRobot = chooseBestRobot(order, bestPlan);

    if (bestRobot == nullptr || bestPlan == nullptr) {
        cout << "No robot can currently complete this plan." << endl;
        return;
    }

    if (confirmPlan(order, bestPlan)) {
        cout << "Plan confirmed and stored." << endl;
    }
}

void PlanManager::displayActivePlans() {
    if (activeHead == nullptr) {
        cout << "No active plans." << endl;
        return;
    }

    PickingPlan* current = activeHead;

    while (current != nullptr) {
        current->display();
        current = current->next;
    }
}

PickingPlan* PlanManager::getActiveHead() const {
    return activeHead;
}

void runPlanningMenu(PlanManager& planner) {
    while (true) {
        cout << "\n===== PLANNING MENU =====" << endl;
        cout << "1. Process next pending order" << endl;
        cout << "2. Display active plans" << endl;
        cout << "0. Back" << endl;

        int choice = readIntInRange("Choice: ", 0, 2);

        if (choice == 0) {
            return;
        }
        else if (choice == 1) {
            planner.processNextOrder();
        }
        else if (choice == 2) {
            planner.displayActivePlans();
        }

        pauseScreen();
    }
}
