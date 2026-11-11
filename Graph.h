#pragma once
#include "common.h"

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
    GraphNode(string id, string name, LocationType type);
	Location location;
	Edge* edgeHead;
	GraphNode* next;
	int dist;
	int hops;
	bool visited;
	GraphNode* parent;
};

class GraphQueue {
private:
	GraphNode* front;
	GraphNode* rear;
public:
	GraphQueue();
	void Enqueue(GraphNode* node);
	GraphNode* Dequeue();
	bool isEmpty();
	~GraphQueue();
};

class Graph {
private:
	GraphNode* vertexHead;
	int vertexCount;
	int edgeCount;

	GraphNode* findVertex(string id);
	void resetWorkingFields();
	void runDijkstra(GraphNode* source);

public:
	// Constructors and destructors
	Graph();
	~Graph();

	// Location operations
	bool addLocation(string id, string name, LocationType type);

	bool locationExists(string id);

	LocationType getLocationType(string id);

	void displayAllLocations();

	// Pathway operations
	bool addPathway(string a, string b, int distance, PathStatus status);

	bool removePathway(string a, string b);

	bool blockPathway(string a, string b, PathStatus& oldStatus);

	bool reopenPathway(string a, string b);

	bool setPathwayStatus(string a, string b, PathStatus status);

	// File loading operations
	bool loadLocations(string file);

	bool loadPathways(string file);

	// Graph traversal
	bool isConnected(string fromId, string toId);

	// Shortest path operations
	RouteResult shortestRoute(string fromId, string toId);

	int shortestDistance(string fromId, string toId);

	bool nearestOfType(string fromId, LocationType type, string& nearestId, int& distance);

	// Route validation
	bool isRouteStillOpen(const RouteResult& route);

    void displayRoute(const RouteResult& route);
	// Getters
	int getVertexCount() const;

	int getEdgeCount() const;
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;
	
};
void runGraphMenu(Graph& graph, UndoManager& undo);
