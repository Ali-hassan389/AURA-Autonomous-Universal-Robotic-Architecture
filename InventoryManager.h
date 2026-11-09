#pragma once 
#include "common.h" 
#include "Graph.h" 
#include "RackRegistry.h" 
#include "ProductAVL.h" 
#include "RackEntry.h" 

class UndoManager; 

//Node class for a single Allocation line
class AllocationLine { 
public: 
    string rackId; 
    int quantity; 
    AllocationLine* next;

    AllocationLine(string r, int q); 
}; 

// This class holds the complete plan for allocating a product
class AllocationResult { 
public:
    AllocationLine* lineHead; 
    int totalAllocated;
    bool shortage; 

    AllocationResult(); 
    ~AllocationResult();
    AllocationResult(AllocationResult& other); 
    void addLine(string r, int q); 
}; 

class InventoryManager { 
private: 
    ProductAVL tree; 
    RackRegistry& racks;

public: 
    InventoryManager(RackRegistry& R); 

 
     // Loads products.txt into the AVL tree
    bool loadProducts(string file);

    // Loads inventory.txt into rack entries
    bool loadInventory(string file); 

    // Product operations
    bool insertProduct(string id, string name, double weight, int reorderLevel); 

    bool deleteProduct(string id); 

    ProductNode* searchProduct(string id);

    bool productExists(string id);

    double getUnitWeight(string id); 

    // Rack entry operations
    bool addRackEntry(string productId, string rackId, int qty); 

    bool setQuantity(string productId, string rackId, int newTotal, int& oldTotal); 

    bool removeRackEntry(string productId, string rackId); 

    void displayRacks(string productId);

    // Inventory queries
    int totalStock(string productId); 

    int totalAvailable(string productId); 

    void displayLowStock(); 

    bool hasReservation(string productId); 

    // Execution functions
    AllocationResult allocate(string productId, int qty, string referenceLocationId, Graph& graph); 
 
    bool reserve(string orderId, string productId, string rackId, int qty); 
    
    void releaseAllReservations(string orderId); 
   
    bool canCollect(string orderId, string productId, string rackId, int qty); 

    bool collect(string orderId, string productId, string rackId, int qty); 

    // Undo helper functions
    bool rackEntryExists(string productId, string rackId); 
    
    int reservedQty(string productId, string rackId); 

    void runInventoryMenu(Graph& graph, UndoManager& undo);
}; 
