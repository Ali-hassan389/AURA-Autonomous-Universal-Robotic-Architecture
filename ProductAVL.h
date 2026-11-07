#pragma once
#include "RackEntry.h"

using namespace std;

// Each node stores one product and connects it to its rack list
struct ProductNode {
    string productId;
    string name;
    double unitWeight;
    int reorderLevel;

    RackEntry* rackHead;
    ProductNode* left;
    ProductNode* right;
    int height;

    // Constructor to initialize a new product node
    ProductNode(string id, string n, double weight, int reorder);
};

class ProductAVL {
private:
    ProductNode* root;

    // Returns the height of a node
    int height(ProductNode* p);

    // Performs single left rotation
    ProductNode* rotateLeft(ProductNode* r); 

    // Performs single right rotation
    ProductNode* rotateRight(ProductNode* r);

    // Performs double left-right rotation
    ProductNode* rotateLeftRight(ProductNode* r);

    // Performs double right-left rotation
    ProductNode* rotateRightLeft(ProductNode* r);

    // Recursively inserts a product according to its ProductID and balances the tree
    ProductNode* insertHelper(ProductNode* newNode, ProductNode* T);

    // Deletes all rack entries belonging to a product
    void deleteRackList(RackEntry* current);

    // Recursively deletes a product and balances the tree
    ProductNode* Delete(string productId, ProductNode* T);

    // Displays products using inorder traversal
    void inorder(ProductNode* node);

    // Deletes the complete tree using postorder traversal
    void destroy(ProductNode* node);

    // Prints the tree sideways, displaying the right subtree above the parent
    void displayHelper(ProductNode* node, int spaces);

public:
    // Initializes an empty AVL tree
    ProductAVL();

    // Searches for a product iteratively using its Productid
    ProductNode* search(string productId);

    // Inserts a new product
    void insert(ProductNode* newNode);

    // Removes a product by its Productid
    bool remove(string productId);

    //  the inorder traversal to display all products
    void inorderApply();

    // Displays the AVL tree sideways and the height of every node
    void displayTree();

    // Prevents memory leaks by deleting the complete tree
    ~ProductAVL();
};