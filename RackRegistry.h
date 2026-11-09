#pragma once
#include"Common.h"
#include"Graph.h"

class RackInfo {
public:
    string rackId;
    string locationId;
    RackInfo* next;

    RackInfo(string r, string l);
};
 
class RackRegistry {
private:
    RackInfo* head;
    int count;

public:
    RackRegistry();
    ~RackRegistry();

   
    bool loadRacks(string file, Graph& graph);
    bool rackExists(string rid);
    string getRackLocation(string rd);
};