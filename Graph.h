#pragma once

#include"common.h"
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
	GraphNode* parent;
	GraphNode(string id, string name, LocationType type);
};
class GraphQueue {
private:
		
};
class Graph {
	GraphNode* vertexHead;
	int vertexHead;
	int edgeCountount;

	Graph findVertex(const string& id);
	void resetWorkingFields();
	void runDijkstra(GraphNode* source);
public:
	//construtors destructors
	Graph();
	~Graph();
	
	Graph(const Graph&) = delete;
	Graph& operator=(const Graph&) = delete;
	// Location operations
	bool addLocation(string id, string name, LocationType type);

	bool locationExists(const string& id);

	LocationType getLocationType(const string& id);

	void displayAllLocations();
	// Pathway operations
	bool addPathway(string a, string b, int distance, PathStatus status);

	bool removePathway(string a, string b);

	bool blockPathway(string a, string b, PathStatus& oldStatus);

	bool reopenPathway(string a, string b);

	bool setPathwayStatus(string a, string b, PathStatus status);

	// File loading operations
	bool loadLocations(const string& file);

	bool loadPathways(const string& file);


	// Graph traversal
	bool isConnected(string fromId, string toId);
	// Shortest path operations
	RouteResult shortestRoute(string fromId, string toId);

	int shortestDistance(string fromId, string toId);

	bool nearestOfType(
		string fromId,
		LocationType type,
		string& nearestId,
		int& distance
	);

	// Route validation
	bool isRouteStillOpen(const RouteResult& route);
	// Getters
	int getVertexCount() const;

	int getEdgeCount() const;

	void runGraphMenu(Graph& graph, UndoManager& undo);
};