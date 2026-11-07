#include "ProductAVL.h"


// Constructor to initialize a new product node.
ProductNode::ProductNode(string id, string n, double weight, int reorder) {
    productId = id;
    name = n;
    unitWeight = weight;
    reorderLevel = reorder;
    rackHead = NULL;
    left = NULL;
    right = NULL;
    height = 0; 
}

// Returns the height of a node
int ProductAVL::height(ProductNode* p) {
    if (p == NULL)
        return -1;
    else
        return p->height;
}

ProductNode* ProductAVL::rotateLeft(ProductNode* r) {
    ProductNode* temp = r->right;

    r->right = temp->left;
    temp->left = r;

    r->height = max(height(r->left), height(r->right)) + 1;
    temp->height = max(height(temp->left), height(temp->right)) + 1;

    return temp;
}

// Performs single right rotation.
ProductNode* ProductAVL::rotateRight(ProductNode* r) {
    ProductNode* temp = r->left;

    r->left = temp->right;
    temp->right = r;

    r->height = max(height(r->left), height(r->right)) + 1;
    temp->height = max(height(temp->left), height(temp->right)) + 1;

    return temp;
}

// Performs double left-right rotation.
ProductNode* ProductAVL::rotateLeftRight(ProductNode* r) {
    r->left = rotateLeft(r->left);
    return rotateRight(r);
}

// Performs double right-left rotation.
ProductNode* ProductAVL::rotateRightLeft(ProductNode* r) {
    r->right = rotateRight(r->right);
    return rotateLeft(r);
}

// Recursively inserts a product according to its ProductID and balances the tree.
ProductNode* ProductAVL::insertHelper(ProductNode* newNode, ProductNode* T) {
    if (T == NULL) {
        T = newNode;
    }
    else if (newNode->productId < T->productId) {
        T->left = insertHelper(newNode, T->left);

        if (height(T->left) - height(T->right) == 2) {
            if (newNode->productId < T->left->productId)
                T = rotateRight(T);
            else
                T = rotateLeftRight(T);
        }
    }
    else if (newNode->productId > T->productId) {
        T->right = insertHelper(newNode, T->right);

        if (height(T->left) - height(T->right) == -2) {
            if (newNode->productId > T->right->productId)
                T = rotateLeft(T);
            else
                T = rotateRightLeft(T);
        }
    }
    T->height = max(height(T->left), height(T->right)) + 1;
    return T;
}

// Deletes all rack entries belonging to a product.
void  ProductAVL::deleteRackList(RackEntry* current) {
    while (current != NULL) {
        RackEntry* next = current->nextRack;
        delete current;
        current = next;
    }
}

// Recursively deletes a product and balances the tree
ProductNode* ProductAVL::Delete(string productId, ProductNode* T) {
    if (T == NULL)
        return T;

    else if (productId < T->productId) {
        T->left = Delete(productId, T->left);

        if (height(T->left) - height(T->right) == -2) {

            if (height(T->right->right) >= height(T->right->left))
                T = rotateLeft(T);
            else
                T = rotateRightLeft(T);
        }
    }

    else if (productId > T->productId) {
        T->right = Delete(productId, T->right);

        if (height(T->left) - height(T->right) == 2) {

            if (height(T->left->left) >= height(T->left->right))
                T = rotateRight(T);
            else
                T = rotateLeftRight(T);
        }
    }

    else {

        if (T->left == NULL) {
            ProductNode* temp = T;
            T = T->right;

            deleteRackList(temp->rackHead);
            temp->rackHead = NULL;
            delete temp;
        }
        else if (T->right == NULL) {
            ProductNode* temp = T;
            T = T->left;

            deleteRackList(temp->rackHead);
            temp->rackHead = NULL;
            delete temp;
        }
        else {
            ProductNode* temp = T->right;
            while (temp->left != NULL)
                temp = temp->left;


            string tempId = temp->productId;
            T->name = temp->name;
            T->unitWeight = temp->unitWeight;
            T->reorderLevel = temp->reorderLevel;
            T->rackHead = temp->rackHead;
            temp->rackHead = NULL;

            T->right = Delete(tempId, T->right);
            T->productId = tempId;
            if (height(T->left) - height(T->right) == 2) {
                if (height(T->left->left) >= height(T->left->right))
                    T = rotateRight(T);
                else
                    T = rotateLeftRight(T);
            }
        }
    }

    if (T != NULL) {
        T->height = max(height(T->left), height(T->right)) + 1;
    }
    return T;
}

// Displays products using inorder traversal.
void ProductAVL::inorder(ProductNode* node) {
    if (node == NULL)
        return;

    inorder(node->left);

    cout << "Product ID: " << node->productId << endl;
    cout << "Name: " << node->name << endl;
    cout << "Weight: " << node->unitWeight << endl;
    cout << "Reorder Level: " << node->reorderLevel << endl;
    cout << "------------------------" << endl;

    inorder(node->right);
}

// Deletes the complete tree using postorder traversal.
void ProductAVL::destroy(ProductNode* node) {
    if (node == NULL)
        return;

    destroy(node->left);
    destroy(node->right);

    deleteRackList(node->rackHead);
    node->rackHead = NULL;

    delete node;
}

// Prints the tree sideways, displaying the right subtree above the parent
void ProductAVL::displayHelper(ProductNode* node, int spaces) {
    if (node == NULL)
        return;

    spaces += 5;

    displayHelper(node->right, spaces);

    cout << endl;

    for (int i = 5; i < spaces; i++)
        cout << " ";

    cout << node->productId << " (H:" << node->height << ")" << endl;

    displayHelper(node->left, spaces);
}

// Initializes an empty AVL tree
ProductAVL::ProductAVL() {
    root = NULL;
}

// Searches for a product iteratively using its Productid
ProductNode* ProductAVL::search(string productId) {
    ProductNode* current = root;

    while (current != NULL) {
        if (productId == current->productId)
            return current;

        else if (productId < current->productId)
            current = current->left;

        else
            current = current->right;
    }

    return NULL;
}

// Inserts a new product
void  ProductAVL::insert(ProductNode* newNode) {
    if (newNode == NULL)
        return;

    if (search(newNode->productId) != NULL) {
        cout << "Product with this ID already exists!" << endl;
        delete newNode;
        return;
    }

    root = insertHelper(newNode, root);
    cout << "Product inserted successfully!" << endl;
}

// Removes a product by its Productid
bool  ProductAVL::remove(string productId) {
    if (search(productId) == NULL) {
        cout << "Product not found!" << endl;
        return false;
    }

    root = Delete(productId, root);
    cout << "Product deleted successfully!" << endl;

    return true;
}

//  the inorder traversal to display all products
void  ProductAVL::inorderApply() {
    if (root == NULL) {
        cout << "No products available!" << endl;
        return;
    }

    inorder(root);
}

// Displays the AVL tree sideways and the height of every node
void  ProductAVL::displayTree() {
    if (root == NULL) {
        cout << "Tree is empty!" << endl;
        return;
    }

    displayHelper(root, 0);
}

// Prevents memory leaks by deleting the complete tree
ProductAVL::~ProductAVL() {
    destroy(root);
    root = NULL;
}