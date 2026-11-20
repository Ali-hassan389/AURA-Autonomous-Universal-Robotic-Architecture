#include "InventoryManager.h" 
#include "UndoManager.h"
#include <iostream> 
#include <fstream> 

// Creates a single allocation entry saying "grab q units from rack r"
AllocationLine::AllocationLine(string r, int q) {
    rackId = r;
    quantity = q;
    next = NULL;
}

// Starts off an empty allocation result with 0 items allocated and no shortage flag
AllocationResult::AllocationResult() {
    lineHead = NULL;
    totalAllocated = 0;
    shortage = false;
}

// Cleans up all the dynamically allocated AllocationLine nodes so memory doesn't leak
AllocationResult::~AllocationResult() {
    AllocationLine* current = lineHead;
    while (current != NULL) {
        AllocationLine* temp = current;
        current = current->next;
        delete temp;
    }
}

// Makes a brand-new, independent copy of another AllocationResult (deep copy)
AllocationResult::AllocationResult(const AllocationResult& other) {
    lineHead = NULL;
    totalAllocated = 0;
    shortage = other.shortage;

    AllocationLine* current = other.lineHead;

    while (current != NULL) {
        addLine(current->rackId, current->quantity);
        current = current->next;
    }
}

// Safely overwrites an existing AllocationResult with another one (deletes old nodes first, then copies new ones)
AllocationResult& AllocationResult::operator=(const AllocationResult& other) {
    if (this == &other) {
        return *this;
    }

    AllocationLine* current = lineHead;

    while (current != NULL) {
        AllocationLine* nextLine = current->next;
        delete current;
        current = nextLine;
    }

    lineHead = NULL;
    totalAllocated = 0;
    shortage = other.shortage;

    current = other.lineHead;

    while (current != NULL) {
        addLine(current->rackId, current->quantity);
        current = current->next;
    }

    return *this;
}

// Adds a new rack pickup line to the end of the AllocationResult list and updates the total count
void AllocationResult::addLine(string r, int q) {
    if (q <= 0) {
        return;
    }
    AllocationLine* newLine = new AllocationLine(r, q);
    if (lineHead == NULL) {
        lineHead = newLine;
    }
    else {
        AllocationLine* temp = lineHead;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newLine;
    }
    totalAllocated += q;
}

// Sets up the InventoryManager by linking it to the warehouse's RackRegistry
InventoryManager::InventoryManager(RackRegistry& R) : racks(R) {
}

// Reads product details (ID, name, weight, reorder level) from a text file and adds them to the product tree
bool InventoryManager::loadProducts(string file) {
    ifstream read(file);
    if (!read.is_open())
        return false;
    string line;
    while (getline(read, line)) {
        line = trimText(line);
        if (line.empty() || line[0] == '#')
            continue;
        string fields[Max_Fields];
        int count = splitLine(line, ',', fields, Max_Fields);
        if (count == 4) {
            // stoi converts string to integer and stod converts string to double
            insertProduct(fields[0], fields[1], stod(fields[2]), stoi(fields[3]));
        }
    }
    read.close();
    return true;
}

// Reads stock quantities per rack from a text file and attaches those rack entries to each product
bool InventoryManager::loadInventory(string file) {
    ifstream read(file);
    if (!read.is_open())
        return false;
    string line;
    while (getline(read, line)) {
        line = trimText(line);
        if (line.empty() || line[0] == '#')
            continue;
        string fields[Max_Fields];
        int count = splitLine(line, ',', fields, Max_Fields);
        if (count == 3) {
            addRackEntry(fields[0], fields[1], stoi(fields[2]));
        }
    }
    read.close();
    return true;
}

// Creates a new product and inserts it into the search tree (fails if the product ID already exists)
bool InventoryManager::insertProduct(string id, string name, double weight, int reorderLevel) {
    string cleanId = toUpperCase(trimText(id));
    if (productExists(cleanId))
        return false;

    ProductNode* newNode = new ProductNode(cleanId, name, weight, reorderLevel);
    tree.insert(newNode);
    return true;
}

// Removes a product from the catalog as long as it exists and isn't currently reserved for any order
bool InventoryManager::deleteProduct(string id) {
    string cleanId = toUpperCase(trimText(id));
    if (!productExists(cleanId))
        return false;
    if (hasReservation(cleanId))
        return false;
    return tree.remove(cleanId);
}

// Looks up a product in the tree by its ID and returns a pointer to it (or NULL if not found)
ProductNode* InventoryManager::searchProduct(string id) {
    string cleanId = toUpperCase(trimText(id));
    return tree.search(cleanId);
}

// Quick helper that returns true if a product ID exists in the system
bool InventoryManager::productExists(string id) {
    return searchProduct(id) != NULL;
}

// Returns the weight of one single unit of a product (or -1 if the product doesn't exist)
double InventoryManager::getUnitWeight(string id) {
    ProductNode* node = searchProduct(id);
    if (node == NULL)
        return -1;
    return node->unitWeight;
}

// Links a warehouse rack to a product with a starting stock quantity (avoids duplicate racks or invalid IDs)
bool InventoryManager::addRackEntry(string productId, string rackId, int qty) {
    ProductNode* node = searchProduct(productId);
    if (node == NULL)
        return false;
    string cleanRack = toUpperCase(trimText(rackId));
    if (!racks.rackExists(cleanRack))
        return false;
    if (qty < 0)
        return false;
    RackEntry* current = node->rackHead;
    while (current != NULL) {
        if (current->getRackId() == cleanRack)
            return false;
        current = current->nextRack;
    }
    RackEntry* newEntry = new RackEntry(cleanRack, qty);
    if (node->rackHead == NULL) {
        node->rackHead = newEntry;
    }
    else {
        RackEntry* temp = node->rackHead;
        while (temp->nextRack != NULL) {
            temp = temp->nextRack;
        }
        temp->nextRack = newEntry;
    }
    return true;
}

// Updates the total stock count of a product on a specific rack and saves the previous count in oldTotal
bool InventoryManager::setQuantity(string productId, string rackId, int newTotal, int& oldTotal) {
    ProductNode* node = searchProduct(productId);
    if (node == NULL)
        return false;
    string cleanRack = toUpperCase(trimText(rackId));
    RackEntry* current = node->rackHead;
    while (current != NULL) {
        if (current->getRackId() == cleanRack) {
            if (newTotal < current->getReservedQty())
                return false;
            oldTotal = current->getTotalQty();
            return current->setTotalQty(newTotal);
        }
        current = current->nextRack;
    }
    return false;
}

// Unlinks and deletes a rack entry from a product (only allowed if nothing is reserved/blocking removal)
bool InventoryManager::removeRackEntry(string productId, string rackId) {
    ProductNode* node = searchProduct(productId);
    if (node == NULL)
        return false;
    string cleanRack = toUpperCase(trimText(rackId));
    RackEntry* current = node->rackHead;
    RackEntry* previous = NULL;
    while (current != NULL) {
        if (current->getRackId() == cleanRack) {
            if (!current->canBeRemoved())
                return false;
            if (previous == NULL) {
                node->rackHead = current->nextRack;
            }
            else {
                previous->nextRack = current->nextRack;
            }
            delete current;
            return true;
        }
        previous = current;
        current = current->nextRack;
    }
    return false;
}

// Prints out every rack storing a given product along with its stock details
void InventoryManager::displayRacks(string productId) {
    ProductNode* node = searchProduct(productId);
    if (node == NULL) {
        cout << "Product not found" << endl;
        return;
    }
    RackEntry* current = node->rackHead;
    while (current != NULL) {
        current->display();
        current = current->nextRack;
    }
}

// Adds up the physical stock of a product across all racks (including reserved items)
int InventoryManager::totalStock(string productId) {
    ProductNode* node = searchProduct(productId);
    if (node == NULL) return 0;
    int sum = 0;
    RackEntry* current = node->rackHead;
    while (current != NULL) {
        sum += current->getTotalQty();
        current = current->nextRack;
    }
    return sum;
}

// Adds up only the free, unreserved units of a product available across all racks
int InventoryManager::totalAvailable(string productId) {
    ProductNode* node = searchProduct(productId);
    if (node == NULL)
        return 0;
    int sum = 0;
    RackEntry* current = node->rackHead;
    while (current != NULL) {
        sum += current->availableQty();
        current = current->nextRack;
    }
    return sum;
}

// Prints all products whose stock has dropped to or below their reorder threshold
void InventoryManager::displayLowStock() {
    cout << "Products at or below reorder level:" << endl;
    cout << "====================================" << endl;
    tree.displayLowStock();
}

// Checks if any rack holding this product currently has units reserved for an order
bool InventoryManager::hasReservation(string productId) {
    ProductNode* node = searchProduct(productId);
    if (node == NULL)
        return false;
    RackEntry* current = node->rackHead;
    while (current != NULL) {
        if (current->getReservedQty() > 0)
            return true;
        current = current->nextRack;
    }
    return false;
}

// Smartly chooses which racks to pull a requested quantity from (prefers racks that can fulfill everything, then closest distance)
AllocationResult InventoryManager::allocate(string productId, int qty, string referenceLocationId, Graph& graph) {
    AllocationResult result;
    if (qty <= 0) {
        result.shortage = true;
        return result;
    }
    ProductNode* product = searchProduct(productId);
    if (product == NULL) {
        result.shortage = true;
        return result;
    }

    int remaining = qty;
    RackEntry* current = product->rackHead;
    while (current != NULL) {
        current->tempTaken = 0;
        current = current->nextRack;
    }
    while (remaining > 0) {
        RackEntry* bestRack = NULL;
        int bestDistance = 2000000000;
        int bestAvailable = -1;
        string bestRackId = "";

        RackEntry* temp = product->rackHead;

        while (temp != NULL) {
            int availableNow = temp->availableQty() - temp->tempTaken;
            if (availableNow > 0) {
                string rackLoc = racks.getRackLocation(temp->getRackId());
                int dist = graph.shortestDistance(referenceLocationId, rackLoc);

                if (dist != -1) {
                    bool candidateCanSupplyAll = availableNow >= remaining;

                    bool currentBestCanSupplyAll =
                        bestRack != NULL && bestAvailable >= remaining;

                    bool isBetter = false;

                    // First candidate is automatically selected.
                    if (bestRack == NULL) {
                        isBetter = true;
                    }
                    // A rack that can supply all remaining units beats
                    // a rack that can only supply part of them.
                    else if (candidateCanSupplyAll && !currentBestCanSupplyAll) {
                        isBetter = true;
                    }
                    // If both candidates have the same supply category,
                    // we apply the tie-breaking rules.
                    else if (candidateCanSupplyAll == currentBestCanSupplyAll) {
                        // Nearest rack first.
                        if (dist < bestDistance) {
                            isBetter = true;
                        }
                        // If distance is equal, choose the rack with more stock.
                        else if (dist == bestDistance && availableNow > bestAvailable) {
                            isBetter = true;
                        }
                        // If everything else is equal, we choose smaller rack ID.
                        else if (
                            dist == bestDistance &&
                            availableNow == bestAvailable &&
                            temp->getRackId() < bestRackId
                            ) {
                            isBetter = true;
                        }
                    }

                    if (isBetter) {
                        bestRack = temp;
                        bestDistance = dist;
                        bestAvailable = availableNow;
                        bestRackId = temp->getRackId();
                    }
                }
            }

            temp = temp->nextRack;
        }


        if (bestRack == NULL) {
            result.shortage = true;
            break;
        }

        int takeQty = remaining;
        if (bestAvailable < remaining) {
            takeQty = bestAvailable;
        }

        result.addLine(bestRack->getRackId(), takeQty);
        bestRack->tempTaken += takeQty;
        remaining -= takeQty;
    }
    return result;
}

// Locks/reserves a specific quantity of a product on a rack for an order so other orders can't steal it
bool InventoryManager::reserve(string orderId, string productId, string rackId, int qty) {
    string cleanOrderId = toUpperCase(trimText(orderId));
    string cleanProductId = toUpperCase(trimText(productId));
    string cleanRackId = toUpperCase(trimText(rackId));

    if (cleanOrderId.empty() || cleanProductId.empty() || cleanRackId.empty()) {
        return false;
    }
    ProductNode* product = searchProduct(cleanProductId);
    if (product == NULL) {
        return false;
    }
    RackEntry* current = product->rackHead;
    while (current != NULL) {
        if (current->getRackId() == cleanRackId) {
            return current->addReservation(cleanOrderId, qty);
        }
        current = current->nextRack;
    }
    return false;
}

// Cancels and frees up all stock reservations tied to a given order across the entire warehouse
void InventoryManager::releaseAllReservations(string orderId) {
    string cleanOrderId = toUpperCase(trimText(orderId));

    if (cleanOrderId.empty()) {
        return;
    }

    tree.releaseReservationsForOrder(cleanOrderId);
}

// Verifies whether a robot is actually allowed to pick up 'qty' reserved units of a product from a rack
bool InventoryManager::canCollect(string orderId, string productId, string rackId, int qty) {
    ProductNode* product = searchProduct(productId);
    if (product == NULL)
        return false;
    RackEntry* current = product->rackHead;
    while (current != NULL) {
        string cleanRackId = toUpperCase(trimText(rackId));

        if (current->getRackId() == cleanRackId) {
            return current->canConsume(orderId, qty);
        }
        current = current->nextRack;
    }
    return false;
}

// Officially deducts the picked-up items from both the order's reservation and the rack's physical stock
bool InventoryManager::collect(string orderId, string productId, string rackId, int qty) {
    ProductNode* product = searchProduct(productId);
    if (product == NULL) return false;
    RackEntry* current = product->rackHead;
    while (current != NULL) {
        string cleanRackId = toUpperCase(trimText(rackId));
        if (current->getRackId() == cleanRackId) {
            return current->consumeForCollection(orderId, qty);
        }
        current = current->nextRack;
    }
    return false;
}

// Checks if a specific product is currently registered on a specific rack
bool InventoryManager::rackEntryExists(string productId, string rackId) {
    ProductNode* product = searchProduct(productId);
    if (product == NULL)
        return false;
    string cleanRack = toUpperCase(trimText(rackId));
    RackEntry* current = product->rackHead;
    while (current != NULL) {
        if (current->getRackId() == cleanRack)
            return true;
        current = current->nextRack;
    }
    return false;
}

// Returns how many units of a product are currently reserved on a specific rack
int InventoryManager::reservedQty(string productId, string rackId) {
    ProductNode* product = searchProduct(productId);
    if (product == NULL)
        return 0;
    string cleanRack = toUpperCase(trimText(rackId));
    RackEntry* current = product->rackHead;
    while (current != NULL) {
        if (current->getRackId() == cleanRack)
            return current->getReservedQty();
        current = current->nextRack;
    }
    return 0;
}

// Looks up which map location/node ID a given rack is physically sitting at
string InventoryManager::getRackLocation(string rackId) {
    return racks.getRackLocation(toUpperCase(trimText(rackId)));
}

// Runs the interactive console menu so the user can view, add, update, or test inventory features
void InventoryManager::runInventoryMenu(Graph& graph, UndoManager& undo) {
    (void)undo;

    while (true) {
        cout << "\n===== INVENTORY MENU =====" << endl;
        cout << "1. Load products" << endl;
        cout << "2. Load inventory" << endl;
        cout << "3. Insert product" << endl;
        cout << "4. Delete product" << endl;
        cout << "5. Display all products" << endl;
        cout << "6. Display product racks" << endl;
        cout << "7. Add rack entry" << endl;
        cout << "8. Remove rack entry" << endl;
        cout << "9. Set rack quantity" << endl;
        cout << "10. Display low stock" << endl;
        cout << "11. Allocate stock" << endl;
        cout << "0. Back" << endl;

        int choice = readIntInRange("Choice: ", 0, 11);

        if (choice == 0) {
            return;
        }

        if (choice == 1) {
            string file = readNonEmptyString("Products file: ");
            cout << (loadProducts(file) ? "Products loaded." : "Products file could not be loaded.") << endl;
        }
        else if (choice == 2) {
            string file = readNonEmptyString("Inventory file: ");
            cout << (loadInventory(file) ? "Inventory loaded." : "Inventory file could not be loaded.") << endl;
        }
        else if (choice == 3) {
            string id = readNonEmptyString("Product ID: ");
            string name = readNonEmptyString("Product name: ");
            double weight = readDoubleInRange("Unit weight: ", 0.01, 100000.0);
            int reorder = readIntInRange("Reorder level: ", 0, 1000000);
            cout << (insertProduct(id, name, weight, reorder)
                ? "Product inserted." : "Product could not be inserted.") << endl;
        }
        else if (choice == 4) {
            string id = readNonEmptyString("Product ID: ");
            cout << (deleteProduct(id)
                ? "Product deleted." : "Product could not be deleted.") << endl;
        }
        else if (choice == 5) {
            tree.inorderApply();
        }
        else if (choice == 6) {
            string id = readNonEmptyString("Product ID: ");
            displayRacks(id);
        }
        else if (choice == 7) {
            string product = readNonEmptyString("Product ID: ");
            string rack = readNonEmptyString("Rack ID: ");
            int quantity = readIntInRange("Total quantity: ", 0, 1000000);
            cout << (addRackEntry(product, rack, quantity)
                ? "Rack entry added." : "Rack entry could not be added.") << endl;
        }
        else if (choice == 8) {
            string product = readNonEmptyString("Product ID: ");
            string rack = readNonEmptyString("Rack ID: ");
            cout << (removeRackEntry(product, rack)
                ? "Rack entry removed." : "Rack entry could not be removed.") << endl;
        }
        else if (choice == 9) {
            string product = readNonEmptyString("Product ID: ");
            string rack = readNonEmptyString("Rack ID: ");
            int newTotal = readIntInRange("New total quantity: ", 0, 1000000);
            int oldTotal = 0;

            if (setQuantity(product, rack, newTotal, oldTotal)) {
                cout << "Quantity updated from " << oldTotal << " to " << newTotal << "." << endl;
                undo.recordQuantityUpdate(product, rack, oldTotal, newTotal);
            }
            else {
                cout << "Quantity could not be updated." << endl;
            }
        }
        else if (choice == 10) {
            displayLowStock();
        }
        else if (choice == 11) {
            string product = readNonEmptyString("Product ID: ");
            int quantity = readIntInRange("Quantity: ", 1, 1000000);
            string location = readNonEmptyString("Reference location: ");
            AllocationResult result = allocate(product, quantity, location, graph);

            if (result.shortage) {
                cout << "Shortage or unreachable rack. Allocated: "
                    << result.totalAllocated << endl;
            }
            else {
                cout << "Allocation:" << endl;
                AllocationLine* line = result.lineHead;
                while (line != nullptr) {
                    cout << "  " << line->rackId << ": " << line->quantity << endl;
                    line = line->next;
                }
            }
        }

        pauseScreen();
    }
}
