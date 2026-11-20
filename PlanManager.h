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

    // Creates a new item to pick up and adds it to the end of our temporary shopping list
    void appendCollectionItem(CollectionItem*& head, CollectionItem*& tail, string rackId, string rackLocationId,
        string productId, int quantity, double unitWeight);

    // Finds which racks hold the ordered products and builds a list of everything the robot needs to grab
    bool allocateAllItems(Order* order, Robot* robot, CollectionItem*& items);

    // Sorts the pickup items into a smart visiting order (like picking the closest rack next)
    bool buildVisitSequence(CollectionItem* items, string startLocation, CollectionItem*& sequence);

    // Calculates the path between two spots and adds that movement step (RouteLeg) to the current trip
    bool appendLeg(PlanTrip* trip, Robot* robot, string fromId, string toId, double carriedWeight);

    // Sends the robot to the packing station to drop off items and finishes up the current trip
    bool closeTrip(PickingPlan* plan, PlanTrip*& trip, Robot* robot, string currentLocation, double carriedWeight);

    // Splits the ordered items into one or more trips so the robot never exceeds its weight limit or battery
    bool buildTrips(PickingPlan* plan, Robot* robot, CollectionItem* sequence);

    // Deletes an entire linked list of PickingPlan objects to prevent memory leaks
    void deletePlanList(PickingPlan* head);

    // Compares two possible plans and returns true if the new one is better (fewer trips, less distance/battery)
    bool isBetterPlan(PickingPlan* candidate, PickingPlan* currentBest);

    // Saves a finalized picking plan into our list of active plans
    void appendActivePlan(PickingPlan* plan);

public:
    // Sets up the PlanManager with references to the warehouse map, inventory, orders, and robots
    PlanManager(Graph& graph, InventoryManager& inventory, OrderManager& orders, RobotManager& robots);

    // Cleans up all active picking plans when the PlanManager is destroyed
    ~PlanManager();

    // Stops anyone from copying the PlanManager (prevents two managers from accidentally sharing/deleting the same plans)
    PlanManager(const PlanManager&) = delete;
    PlanManager& operator=(const PlanManager&) = delete;

    // Checks if the warehouse actually has enough items in stock to fulfill the order
    bool checkStock(Order* order);

    // Picks the next waiting order from the queue that is ready to be planned
    Order* selectNextOrder();

    // Tries to create a complete step-by-step picking route for a specific robot to handle an order
    PickingPlan* buildPlanForRobot(Order* order, Robot* robot, bool partialAllowed);

    // Tests all available robots on an order and picks the best robot along with its best plan
    Robot* chooseBestRobot(Order* order, PickingPlan*& bestPlan);

    // Prints out all the details of a picking plan on the screen
    void displayPlan(PickingPlan* plan);

    // Locks in the plan: reserves the stock, assigns the robot, and saves the plan as active
    bool confirmPlan(Order* order, PickingPlan* plan);

    // Searches the active plans list to find the plan matching a specific order ID
    PickingPlan* findPlan(string orderId);

    // Cancels and deletes an active plan (and frees up its resources) using its order ID
    void discardPlan(string orderId);

    // Main workflow function: grabs the next order, finds the best robot/plan for it, and processes it
    void processNextOrder();

    // Prints out all currently active picking plans in the system
    void displayActivePlans();

    // Returns the pointer to the first active plan in the linked list
    PickingPlan* getActiveHead() const;
};

// Runs the interactive console menu where the user can create, view, or manage picking plans
void runPlanningMenu(PlanManager& planner);
