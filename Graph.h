#pragma once
#include "Common.h"

class UndoManager;

class Location {
public:
    string id;
    string name;
    LocationType type;
    Location(string locationId, string locationName, LocationType locationType);
};

class Edge {
public:
    string destinationId;
    int distance;
    PathStatus status;
    Edge* next;
    Edge(string destination, int edgeDistance, PathStatus edgeStatus);
};

class GraphNode {
public:
    Location location;
    Edge* edgeHead;
    GraphNode* next;
    int dist;
    int hops;
    bool visited;
    GraphNode* parent;
    GraphNode(string id, string name, LocationType type);
};

class Graph {
private:
    GraphNode* vertexHead;
    int vertexCount;
    int edgeCount;

    GraphNode* findVertex(const string& id);
    void resetWorkingFields();
    void runDijkstra(GraphNode* source);

public:
    Graph();
    ~Graph();
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    bool addLocation(const string& id, const string& name, LocationType type);
    bool locationExists(const string& id);
    LocationType getLocationType(const string& id);
    void displayAllLocations();
    void displayLocationInfo(const string& id);

    bool addPathway(const string& a, const string& b, int distance, PathStatus status);
    bool removePathway(const string& a, const string& b);
    bool blockPathway(const string& a, const string& b, PathStatus& oldStatus);
    bool reopenPathway(const string& a, const string& b);
    bool setPathwayStatus(const string& a, const string& b, PathStatus status);

    bool loadLocations(const string& file);
    bool loadPathways(const string& file);
    bool isConnected(const string& fromId, const string& toId);
    RouteResult shortestRoute(const string& fromId, const string& toId);
    int shortestDistance(const string& fromId, const string& toId);
    bool nearestOfType(const string& fromId, LocationType type, string& nearestId, int& distance);
    bool isRouteStillOpen(const RouteResult& route);
    void displayRoute(const RouteResult& route);

    int getVertexCount() const;
    int getEdgeCount() const;
};

void runGraphMenu(Graph& graph, UndoManager& undo);
