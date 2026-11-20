#include "WarehouseSystem.h"

#include <iostream>

using namespace std;

WarehouseSystem::WarehouseSystem()
    : graph(),
      racks(),
      inventory(racks),
      orders(inventory),
      robots(graph),
      undo(graph, inventory, robots),
      planner(graph, inventory, orders, robots),
      executor(planner, graph, inventory, orders, robots),
      charging(graph, robots),
      dataLoaded(false) {
}

bool WarehouseSystem::loadData(const string& folder) {
    string locationsFile = folder + "/locations.txt";
    string pathwaysFile = folder + "/pathways.txt";
    string racksFile = folder + "/racks.txt";
    string productsFile = folder + "/products.txt";
    string inventoryFile = folder + "/inventory.txt";
    string ordersFile = folder + "/orders.txt";
    string robotsFile = folder + "/robots.txt";
    string chargingFile = folder + "/charging_stations.txt";

    bool locationsOk = graph.loadLocations(locationsFile);
    bool pathwaysOk = graph.loadPathways(pathwaysFile);
    bool racksOk = racks.loadRacks(racksFile, graph);
    bool productsOk = inventory.loadProducts(productsFile);
    bool inventoryOk = inventory.loadInventory(inventoryFile);
    bool ordersOk = orders.loadOrders(ordersFile);
    bool robotsOk = robots.loadRobots(robotsFile);
    bool chargingOk = charging.loadStations(chargingFile);

    if (locationsOk && pathwaysOk && racksOk && productsOk && inventoryOk &&
        ordersOk && robotsOk && chargingOk) {
        orders.enqueuePendingOrders();
        dataLoaded = true;
        cout << "All warehouse data loaded successfully.\n";
        return true;
    }

    cout << "Some data files could not be loaded. Check the messages above.\n";
    dataLoaded = false;
    return false;
}

void WarehouseSystem::showWelcome() const {
    cout << "\n==============================================\n";
    cout << "        PAKISTAN WAREHOUSE SYSTEM\n";
    cout << "==============================================\n";
    cout << "Products include dairy milk, Lays, Rooh Afza and other local stock.\n";
}

void WarehouseSystem::run() {
    if (!dataLoaded) {
        cout << "The system cannot start because the data was not loaded.\n";
        return;
    }

    int choice;
    do {
        cout << "\n=============== MAIN MENU ===============\n";
        cout << "1. Warehouse layout and pathways\n";
        cout << "2. Inventory management\n";
        cout << "3. Order management\n";
        cout << "4. Robot management\n";
        cout << "5. Planning and reservations\n";
        cout << "6. Execute assigned orders\n";
        cout << "7. Charging management\n";
        cout << "8. Undo history\n";
        cout << "0. Exit\n";

        choice = readIntInRange("Enter choice: ", 0, 8);

        if (choice == 1)
            runGraphMenu(graph, undo);
        else if (choice == 2)
            inventory.runInventoryMenu(graph, undo);
        else if (choice == 3)
            runOrderMenu(orders, inventory, robots);
        else if (choice == 4)
            runRobotMenu(robots, graph, undo);
        else if (choice == 5)
            runPlanningMenu(planner);
        else if (choice == 6)
            runExecutionMenu(executor);
        else if (choice == 7)
            runChargingMenu(charging, robots);
        else if (choice == 8)
            runUndoMenu(undo);
        else if (choice == 0)
            cout << "Closing warehouse system. Goodbye.\n";

    } while (choice != 0);
}
