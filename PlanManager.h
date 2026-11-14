#pragma once

#include "Graph.h"
#include "InventoryManager.h"
#include "OrderManager.h"
#include "PickingPlan.h"
#include "RobotManager.h"

class PlanManager {
private:
    Graph& graph;
    InventoryManager& inventory;
    OrderManager& orders;
    RobotManager& robots;
    PickingPlan* activeHead;

    int noAvailableCount;
    int eligibilityCount;
    int capacityCount;
    int batteryCount;
    int unreachableCount;

    struct CollectionItem {
        string rackId;
        string rackLocationId;
        string productId;
        int quantity;
        double unitWeight;
        bool visited;
        CollectionItem* next;
    };

    void deleteCollectionItems(CollectionItem* head);

    void appendCollectionItem(CollectionItem*& head, CollectionItem*& tail,string rackId, string rackLocationId,
                               string productId, int quantity,double unitWeight);
    bool allocateAllItems(Order* order, Robot* robot,CollectionItem*& items);

    bool buildVisitSequence(CollectionItem* items, string startLocation,CollectionItem*& sequence);

    bool appendLeg(PlanTrip* trip, Robot* robot, string fromId, string toId, double carriedWeight);

    bool closeTrip(PickingPlan* plan, PlanTrip*& trip, Robot* robot,string currentLocation, double carriedWeight);

    bool buildTrips(PickingPlan* plan, Robot* robot,CollectionItem* sequence);

    void deletePlanList(PickingPlan* head);

    bool isBetterPlan(PickingPlan* candidate, PickingPlan* currentBest);

    void appendActivePlan(PickingPlan* plan);

public:
    PlanManager(Graph& graph, InventoryManager& inventory,OrderManager& orders, RobotManager& robots);
    ~PlanManager();

    PlanManager(const PlanManager&) = delete;
    PlanManager& operator=(const PlanManager&) = delete;

    bool checkStock(Order* order);
    Order* selectNextOrder();
    PickingPlan* buildPlanForRobot(Order* order, Robot* robot,bool partialAllowed);
    Robot* chooseBestRobot(Order* order, PickingPlan*& bestPlan);

    void displayPlan(PickingPlan* plan);
    bool confirmPlan(Order* order, PickingPlan* plan);

    PickingPlan* findPlan(string orderId);
    void discardPlan(string orderId);
    void processNextOrder();
    void displayActivePlans();

    PickingPlan* getActiveHead() const;
};

void runPlanningMenu(PlanManager& planner);
