#include "RackRegistry.h"
#include <iostream>
#include <fstream>
using namespace std;

// RackInfo Constructor
RackInfo::RackInfo(string r, string l) {
    rackId = r;
    locationId = l;
    next = NULL;
}

// RackRegistry Constructor
RackRegistry::RackRegistry() {
    head = NULL;
    count = 0;
}
// Destructor
RackRegistry::~RackRegistry() {
    RackInfo* current = head;
    while (current != NULL) {
        RackInfo* temp= current;
        current = current->next;
        delete temp;
    }
}

bool RackRegistry::loadRacks(string file, Graph& graph) {
    ifstream read(file);
    if (!read.is_open()) {
        cout << "Could not open file " << endl;
        return false;
    }
    string line;
    int lineNumber = 0;

    while (getline(read, line)) {
        lineNumber++;
        line = trimText(line);

        // Skip blank lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        string fields[Max_Fields];
        int fieldCount = splitLine(line, ',', fields, Max_Fields);

        if (fieldCount != 2) {
            cout << "Line " << lineNumber << " : Incorrect field count " << endl;
            continue;
        }

        string r = toUpperCase(trimText(fields[0]));
        string l = toUpperCase(trimText(fields[1]));

        //  Unique Rack ID
        if (rackExists(r) ){
            cout << "Line " << lineNumber << " : Duplicate RackID " << r << endl;
            continue;
        }

        // Location exists in the Graph
        if (!graph.locationExists(l)) {
            cout << "Line " << lineNumber << " : Location " << l << " does not exist in graph" << endl;
            continue;
        }

        //  Location is of type RACK
        if (graph.getLocationType(l) != Location_Rack) {
            cout << "Line " << lineNumber << " : Location " << l << " is not a RACK type " << endl;
            continue;
        }

        // Create and add the new RackInfo node to the linked list
        RackInfo* newRack = new RackInfo(r, l);

        if (head == nullptr) {
            head = newRack;
        }
        else {
            RackInfo* temp = head;
            while (temp->next != nullptr) {
                temp = temp->next;
            }
            temp->next = newRack;
        }
        count++;
    }
    read.close();
    return true;
}

bool RackRegistry::rackExists(string rackId) {
    string searchId = toUpperCase(rackId);
    RackInfo* current = head;
    while (current != nullptr) {
        if (current->rackId == searchId) {
            return true;
        }
        current = current->next;
    }
    return false;
}

string RackRegistry::getRackLocation(string rackId) {
    string searchId = toUpperCase(rackId);
    RackInfo* current = head;
    while (current != nullptr) {
        if (current->rackId == searchId) {
            return current->locationId;
        }
        current = current->next;
    }
    return "";
}