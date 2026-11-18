#include "Graph.h"
#include "UndoManager.h"

#include <fstream>
#include <iostream>
#include <climits>

using namespace std;

// The BFS queue is intentionally private to this source file.
class GraphQueue {
private:
    struct QueueNode {
        GraphNode* vertex;
        QueueNode* next;
        QueueNode(GraphNode* value) : vertex(value), next(nullptr) {}
    };
    QueueNode* front;
    QueueNode* rear;
public:
    GraphQueue() : front(nullptr), rear(nullptr) {}
    ~GraphQueue() { while (front) { QueueNode* old = front; front = front->next; delete old; } rear = nullptr; }
    bool isEmpty() const { return front == nullptr; }
    void enqueue(GraphNode* vertex) {
        QueueNode* node = new QueueNode(vertex);
        if (!rear) front = rear = node;
        else { rear->next = node; rear = node; }
    }
    GraphNode* dequeue() {
        if (!front) return nullptr;
        QueueNode* old = front;
        GraphNode* result = old->vertex;
        front = front->next;
        if (!front) rear = nullptr;
        delete old;
        return result;
    }
};

static Edge* findEdge(GraphNode* vertex, const string& destination) {
    if (!vertex) return nullptr;
    for (Edge* edge = vertex->edgeHead; edge; edge = edge->next)
        if (edge->destinationId == destination) return edge;
    return nullptr;
}

static bool unlinkEdge(GraphNode* vertex, const string& destination) {
    if (!vertex) return false;
    Edge* current = vertex->edgeHead;
    Edge* previous = nullptr;
    while (current) {
        if (current->destinationId == destination) {
            if (previous) previous->next = current->next;
            else vertex->edgeHead = current->next;
            delete current;
            return true;
        }
        previous = current;
        current = current->next;
    }
    return false;
}

static void appendRouteFromParents(GraphNode* node, RouteResult& route) {
    if (!node) return;
    appendRouteFromParents(node->parent, route);
    route.append(node->location.id);
}

Location::Location(string locationId, string locationName, LocationType locationType)
    : id(toUpperCase(trimText(locationId))), name(trimText(locationName)), type(locationType) {}

Edge::Edge(string destination, int edgeDistance, PathStatus edgeStatus)
    : destinationId(toUpperCase(trimText(destination))), distance(edgeDistance), status(edgeStatus), next(nullptr) {}

GraphNode::GraphNode(string id, string name, LocationType type)
    : location(id, name, type), edgeHead(nullptr), next(nullptr), dist(INT_MAX), hops(INT_MAX), visited(false), parent(nullptr) {}

Graph::Graph() : vertexHead(nullptr), vertexCount(0), edgeCount(0) {}

Graph::~Graph() {
    while (vertexHead) {
        GraphNode* vertex = vertexHead;
        vertexHead = vertexHead->next;
        while (vertex->edgeHead) {
            Edge* edge = vertex->edgeHead;
            vertex->edgeHead = edge->next;
            delete edge;
        }
        delete vertex;
    }
    vertexCount = edgeCount = 0;
}

GraphNode* Graph::findVertex(const string& rawId) {
    string id = toUpperCase(trimText(rawId));
    for (GraphNode* current = vertexHead; current; current = current->next)
        if (current->location.id == id) return current;
    return nullptr;
}

bool Graph::addLocation(const string& rawId, const string& rawName, LocationType type) {
    string id = toUpperCase(trimText(rawId));
    string name = trimText(rawName);
    if (id.empty() || name.empty() || findVertex(id)) return false;
    GraphNode* node = new GraphNode(id, name, type);
    if (!vertexHead) vertexHead = node;
    else {
        GraphNode* tail = vertexHead;
        while (tail->next) tail = tail->next;
        tail->next = node;
    }
    ++vertexCount;
    return true;
}

bool Graph::locationExists(const string& id) { return findVertex(id) != nullptr; }

LocationType Graph::getLocationType(const string& id) {
    GraphNode* node = findVertex(id);
    return node ? node->location.type : static_cast<LocationType>(-1);
}

bool Graph::addPathway(const string& rawA, const string& rawB, int distance, PathStatus status) {
    string a = toUpperCase(trimText(rawA)), b = toUpperCase(trimText(rawB));
    GraphNode* first = findVertex(a);
    GraphNode* second = findVertex(b);
    if (!first || !second || a == b || distance <= 0 || findEdge(first, b) || findEdge(second, a)) return false;
    Edge* forward = new Edge(b, distance, status);
    Edge* reverse = new Edge(a, distance, status);
    forward->next = first->edgeHead;
    first->edgeHead = forward;
    reverse->next = second->edgeHead;
    second->edgeHead = reverse;
    ++edgeCount;
    return true;
}

bool Graph::removePathway(const string& rawA, const string& rawB) {
    string a = toUpperCase(trimText(rawA)), b = toUpperCase(trimText(rawB));
    GraphNode* first = findVertex(a); GraphNode* second = findVertex(b);
    if (!first || !second || !findEdge(first, b) || !findEdge(second, a)) return false;
    unlinkEdge(first, b); unlinkEdge(second, a); --edgeCount;
    return true;
}

bool Graph::blockPathway(const string& rawA, const string& rawB, PathStatus& oldStatus) {
    string a = toUpperCase(trimText(rawA)), b = toUpperCase(trimText(rawB));
    GraphNode* first = findVertex(a); GraphNode* second = findVertex(b);
    Edge* one = findEdge(first, b); Edge* two = findEdge(second, a);
    if (!one || !two) return false;
    oldStatus = one->status;
    one->status = two->status = Path_Blocked;
    return true;
}

bool Graph::reopenPathway(const string& a, const string& b) { return setPathwayStatus(a, b, Path_Open); }

bool Graph::setPathwayStatus(const string& rawA, const string& rawB, PathStatus status) {
    string a = toUpperCase(trimText(rawA)), b = toUpperCase(trimText(rawB));
    Edge* one = findEdge(findVertex(a), b); Edge* two = findEdge(findVertex(b), a);
    if (!one || !two) return false;
    one->status = two->status = status;
    return true;
}

bool Graph::loadLocations(const string& file) {
    ifstream input(file);
    if (!input.is_open()) { cout << "Could not open locations file: " << file << '\n'; return false; }
    string line; int lineNumber = 0;
    while (getline(input, line)) {
        ++lineNumber; line = trimText(line);
        if (line.empty() || line[0] == '#') continue;
        string fields[8]; int count = splitLine(line, ',', fields, 8);
        LocationType type;
        if (count != 3 || !parseLocationType(fields[1], type) || !addLocation(fields[0], fields[2], type))
            cout << "Warning: invalid locations.txt line " << lineNumber << ".\n";
    }
    return true;
}

bool Graph::loadPathways(const string& file) {
    ifstream input(file);
    if (!input.is_open()) { cout << "Could not open pathways file: " << file << '\n'; return false; }
    string line; int lineNumber = 0;
    while (getline(input, line)) {
        ++lineNumber; line = trimText(line);
        if (line.empty() || line[0] == '#') continue;
        string fields[8]; int count = splitLine(line, ',', fields, 8);
        PathStatus status; int distance = 0; bool validNumber = false;
        if (count == 4) {
            try { size_t used = 0; distance = stoi(fields[2], &used); validNumber = used == fields[2].length(); }
            catch (...) { validNumber = false; }
        }
        if (count != 4 || !validNumber || !parsePathStatus(fields[3], status) ||
            !addPathway(fields[0], fields[1], distance, status))
            cout << "Warning: invalid pathways.txt line " << lineNumber << ".\n";
    }
    return true;
}

void Graph::displayAllLocations() {
    if (!vertexHead) { cout << "No locations loaded.\n"; return; }
    for (GraphNode* vertex = vertexHead; vertex; vertex = vertex->next) {
        cout << "\n" << vertex->location.id << " | " << vertex->location.name << " | "
             << locationTypeToText(vertex->location.type) << '\n';
        for (Edge* edge = vertex->edgeHead; edge; edge = edge->next)
            cout << "  --" << edge->distance << "--> " << edge->destinationId
                 << " [" << pathStatusToText(edge->status) << "]\n";
    }
}

void Graph::displayLocationInfo(const string& id) {
    GraphNode* vertex = findVertex(id);
    if (!vertex) { cout << "Location " << toUpperCase(id) << " does not exist.\n"; return; }
    cout << vertex->location.id << " | " << vertex->location.name << " | "
         << locationTypeToText(vertex->location.type) << '\n';
    cout << "Connections:\n";
    if (!vertex->edgeHead) cout << "  None\n";
    for (Edge* edge = vertex->edgeHead; edge; edge = edge->next)
        cout << "  " << edge->destinationId << " (" << edge->distance << ", "
             << pathStatusToText(edge->status) << ")\n";
}

void Graph::resetWorkingFields() {
    for (GraphNode* node = vertexHead; node; node = node->next) {
        node->dist = INT_MAX; node->hops = INT_MAX; node->visited = false; node->parent = nullptr;
    }
}

bool Graph::isConnected(const string& fromId, const string& toId) {
    GraphNode* source = findVertex(fromId); GraphNode* target = findVertex(toId);
    if (!source || !target) return false;
    if (source == target) return true;
    resetWorkingFields();
    GraphQueue queue;
    source->visited = true; queue.enqueue(source);
    while (!queue.isEmpty()) {
        GraphNode* current = queue.dequeue();
        for (Edge* edge = current->edgeHead; edge; edge = edge->next) {
            if (edge->status != Path_Open) continue;
            GraphNode* neighbour = findVertex(edge->destinationId);
            if (neighbour && !neighbour->visited) {
                neighbour->visited = true;
                if (neighbour == target) return true;
                queue.enqueue(neighbour);
            }
        }
    }
    return false;
}

void Graph::runDijkstra(GraphNode* source) {
    resetWorkingFields();
    if (!source) return;
    source->dist = 0; source->hops = 1;
    while (true) {
        GraphNode* current = nullptr;
        for (GraphNode* node = vertexHead; node; node = node->next) {
            if (node->visited || node->dist == INT_MAX) continue;
            if (!current || node->dist < current->dist ||
                (node->dist == current->dist && node->hops < current->hops) ||
                (node->dist == current->dist && node->hops == current->hops && node->location.id < current->location.id))
                current = node;
        }
        if (!current) break;
        current->visited = true;
        for (Edge* edge = current->edgeHead; edge; edge = edge->next) {
            if (edge->status != Path_Open) continue;
            GraphNode* neighbour = findVertex(edge->destinationId);
            if (!neighbour || neighbour->visited || current->dist > INT_MAX - edge->distance) continue;
            int newDistance = current->dist + edge->distance;
            int newHops = current->hops + 1;
            bool better = newDistance < neighbour->dist ||
                (newDistance == neighbour->dist && newHops < neighbour->hops) ||
                (newDistance == neighbour->dist && newHops == neighbour->hops &&
                 (!neighbour->parent || current->location.id < neighbour->parent->location.id));
            if (better) { neighbour->dist = newDistance; neighbour->hops = newHops; neighbour->parent = current; }
        }
    }
}

RouteResult Graph::shortestRoute(const string& fromId, const string& toId) {
    RouteResult result;
    GraphNode* source = findVertex(fromId); GraphNode* target = findVertex(toId);
    if (!source || !target) {
        if (!source) cout << "Location " << toUpperCase(fromId) << " does not exist.\n";
        if (!target) cout << "Location " << toUpperCase(toId) << " does not exist.\n";
        return result;
    }
    runDijkstra(source);
    if (target->dist == INT_MAX) return result;
    appendRouteFromParents(target, result);
    result.totalDistance = target->dist;
    result.found = true;
    return result;
}

int Graph::shortestDistance(const string& fromId, const string& toId) {
    GraphNode* source = findVertex(fromId); GraphNode* target = findVertex(toId);
    if (!source || !target) return -1;
    runDijkstra(source);
    return target->dist == INT_MAX ? -1 : target->dist;
}

bool Graph::nearestOfType(const string& fromId, LocationType type, string& nearestId, int& distance) {
    GraphNode* source = findVertex(fromId);
    if (!source) return false;
    runDijkstra(source);
    GraphNode* best = nullptr;
    for (GraphNode* node = vertexHead; node; node = node->next) {
        if (node->location.type != type || node->dist == INT_MAX) continue;
        if (!best || node->dist < best->dist || (node->dist == best->dist && node->location.id < best->location.id)) best = node;
    }
    if (!best) return false;
    nearestId = best->location.id; distance = best->dist;
    return true;
}

bool Graph::isRouteStillOpen(const RouteResult& route) {
    if (!route.found || !route.head) return false;
    RouteNode* current = route.head;
    while (current && current->next) {
        Edge* edge = findEdge(findVertex(current->locationId), current->next->locationId);
        if (!edge || edge->status != Path_Open) return false;
        current = current->next;
    }
    return true;
}

void Graph::displayRoute(const RouteResult& route) {
    if (!route.found || !route.head) { cout << "No route exists.\n"; return; }
    RouteNode* node = route.head;
    while (node) { cout << node->locationId; if (node->next) cout << " -> "; node = node->next; }
    cout << " (total distance " << route.totalDistance << ")\n";
}

int Graph::getVertexCount() const { return vertexCount; }
int Graph::getEdgeCount() const { return edgeCount; }

void runGraphMenu(Graph& graph, UndoManager& undo) {
    int choice;
    do {
        cout << "\n========== WAREHOUSE LAYOUT AND PATHWAYS ==========\n"
             << "1. Display all locations and connections\n2. Add location\n3. Add pathway\n"
             << "4. Remove pathway\n5. Block pathway\n6. Reopen pathway\n7. Search location by ID\n"
             << "8. Check connectivity (BFS)\n9. Find shortest route\n10. Find shortest distance\n0. Back\n";
        choice = readIntInRange("Enter choice: ", 0, 10);
        if (choice == 1) graph.displayAllLocations();
        else if (choice == 2) {
            string id = toUpperCase(readNonEmptyString("Location ID: "));
            string name = readNonEmptyString("Location name: ");
            cout << "1. RACK  2. PACKING  3. CHARGING\n";
            int typeChoice = readIntInRange("Type: ", 1, 3);
            LocationType type = typeChoice == 1 ? Location_Rack : (typeChoice == 2 ? Location_Packing : Location_Charging);
            cout << (graph.addLocation(id, name, type) ? "Location added.\n" : "Could not add location (duplicate or invalid data).\n");
        }
        else if (choice == 3) {
            string a = toUpperCase(readNonEmptyString("First location ID: "));
            string b = toUpperCase(readNonEmptyString("Second location ID: "));
            int distance = readIntInRange("Distance (>0): ", 1, INT_MAX);
            cout << "1. OPEN  2. BLOCKED\n";
            PathStatus status = readIntInRange("Status: ", 1, 2) == 1 ? Path_Open : Path_Blocked;
            cout << (graph.addPathway(a, b, distance, status) ? "Pathway added.\n" : "Could not add pathway. Check IDs, distance and duplicates.\n");
        }
        else if (choice == 4) {
            string a = toUpperCase(readNonEmptyString("First location ID: "));
            string b = toUpperCase(readNonEmptyString("Second location ID: "));
            cout << (graph.removePathway(a, b) ? "Pathway removed.\n" : "Pathway not found.\n");
        }
        else if (choice == 5) {
            string a = toUpperCase(readNonEmptyString("First location ID: "));
            string b = toUpperCase(readNonEmptyString("Second location ID: "));
            PathStatus oldStatus;
            if (graph.blockPathway(a, b, oldStatus)) { undo.recordBlock(a, b, oldStatus); cout << "Pathway blocked.\n"; }
            else cout << "Pathway not found.\n";
        }
        else if (choice == 6) {
            string a = toUpperCase(readNonEmptyString("First location ID: "));
            string b = toUpperCase(readNonEmptyString("Second location ID: "));
            cout << (graph.reopenPathway(a, b) ? "Pathway reopened.\n" : "Pathway not found.\n");
        }
        else if (choice == 7) graph.displayLocationInfo(toUpperCase(readNonEmptyString("Location ID: ")));
        else if (choice == 8) {
            string a = toUpperCase(readNonEmptyString("Start location ID: "));
            string b = toUpperCase(readNonEmptyString("Destination location ID: "));
            if (!graph.locationExists(a) || !graph.locationExists(b)) cout << "One or both locations do not exist.\n";
            else cout << (graph.isConnected(a, b) ? "A route exists.\n" : "No open route exists.\n");
        }
        else if (choice == 9) {
            string a = toUpperCase(readNonEmptyString("Start location ID: "));
            string b = toUpperCase(readNonEmptyString("Destination location ID: "));
            RouteResult route = graph.shortestRoute(a, b);
            graph.displayRoute(route);
        }
        else if (choice == 10) {
            string a = toUpperCase(readNonEmptyString("Start location ID: "));
            string b = toUpperCase(readNonEmptyString("Destination location ID: "));
            int distance = graph.shortestDistance(a, b);
            if (distance < 0) cout << "No route exists or a location does not exist.\n";
            else cout << "Shortest distance: " << distance << '\n';
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}
