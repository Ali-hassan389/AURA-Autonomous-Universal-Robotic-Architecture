#pragma once
#include<string>
#include<iostream>
using namespace std;
// These enums we declared will be used throughtout the files
enum LocationType {
    Location_Rack,
    Location_Packing,
    Location_Charging
};
enum PathStatus {
	Path_Open,
	Path_Blocked
};
enum OrderStatus {
	Order_Pending,
	Order_Reserved,
	Order_Assigned,
	Order_PartiallyFulfilled,
	Order_Completed,
	Order_Cancelled
};
enum RobotStatus {
	Robot_Available,
	Robot_Assigned,
	Robot_WaitingforCharging,
	Robot_Charging,
	Robot_UnAvailable
};
enum RobotType{
	Type_Picker,
	Type_Carrier,
	Type_Heavy
};
//These Constants will be used througout the files
double Rate_Picker = 0.20;
double Rate_Carrier = 0.25;
double Rate_Heavy = 0.35;

double Safety_Reserve = 10.0;
double Low_Battery_Limit = 20.0;
double Full_Battery = 100.0;

int Picker_MaxCapacity = 15;
int Carrier_MinCapacity = 15;
int Carrier_MaxCapacity = 30;
int Heavy_MaxCapacity = 60;

int Picker_MaxUnitWeight = 5;
int Heavy_MinOrderWeight = 15;
int Heavy_MinUnitWeight = 10;

int MaxSlots_PerStation = 10;
int Max_Fields = 8;


//String helper functions 
string toUpperCase(const string& str) {
	string result = str;
	for (int i = 0; i < (int)result.length(); i++) {
		if (result[i] >= 'a' && result[i] <= 'z') {
			result[i] = result[i] - 32;
		}
	}
	return result;
}
// uses 2 pointers to check spaces one from nonempty space to left and then from right
string trimText(const string& str) {
    int start = 0;
    int end = (int)str.length() - 1;
    while (start <= end) {
        if (str[start] == ' ' || str[start] == '\t' || str[start] == '\n' || str[start] == '\r') {
            start++;
        }
        else {
            break;
        }
    }
    if (start > end) {
        return "";
    }
    while (end >= start) {
        if (str[end] == ' ' || str[end] == '\t' || str[end] == '\n' || str[end] == '\r') {
            end--;
        }
        else {
            break;
        }
    }
    string result = "";
    for (int i = start; i <= end; i++) {
        result += str[i];
    }
    return result;
}
int splitLine(const string& line, char delimiter, string fields[], int maxFields) {
    if (line.empty() || maxFields <= 0) 
        return 0;

    int count = 0;
    string currentField = "";

    for (int i = 0; i < (int)line.length(); i++) {
        if (line[i] == delimiter) {
            if (count < maxFields) {
                fields[count] = trimText(currentField);
                count++;
            }
            currentField = "";
        }
        else {
            currentField += line[i];
        }
    }
    if (count < maxFields) {
        fields[count] = trimText(currentField);
        count++;
    }

    return count;
}

//  Input Validation helper functions
// Read an integer within a range
int readIntInRange(const string& prompt, int low, int high) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            if (value >= low && value <= high) {
                cin.ignore(10000, '\n');
                return value;
            }
            else {
                cout << "Please enter a number between " << low << " and " << high << ".\n";
            }
        }
        else {
            cout << "Invalid input. Please enter a valid integer.\n";
            cin.clear();         
            cin.ignore(10000, '\n'); 
        }
    }
}

//Read a double within a range
double readDoubleInRange(const string& prompt, double low, double high) {
    double value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            if (value >= low && value <= high) {
                cin.ignore(10000, '\n');
                return value;
            }
            else {
                cout << "Please enter a value between " << low << " and " << high << ".\n";
            }
        }
        else {
            cout << "Invalid input. Please enter a valid number.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
}

// Read a non-empty string
string readNonEmptyString(const string& prompt) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        input = trimText(input); 

        if (!input.empty()) {
            return input;
        }
        cout << "Input cannot be empty. Please try again.\n";
    }
}

//Read Yes/No response
bool readYesNo(const string& prompt) {
    while (true) {
        string response = readNonEmptyString(prompt + " (y/n): ");
        response = toUpperCase(response); 

        if (response == "Y" || response == "YES") {
            return true;
        }
        if (response == "N" || response == "NO") {
            return false;
        }
        cout << "Please type 'y' or 'n'.\n";
    }
}

// Pause the screen until Enter is pressed
void pauseScreen() {
    cout << "Press Enter to continue...";
    string dummy;
    getline(cin, dummy);
}

// enums to string fucnctions
string locationTypeToText(LocationType type) {
    switch (type) {
    case Location_Rack:     
        return "RACK";
    case Location_Packing: 
        return "PACKING";
    case Location_Charging: 
        return "CHARGING";
    }
    return "UNKNOWN";
}
bool parseLocationType(string text, LocationType& outType) {
    text = toUpperCase(text);
    if (text == "RACK") { outType = Location_Rack; 
    return true; }
    if (text == "PACKING") { outType = Location_Packing; 
    return true; }
    if (text == "CHARGING") { outType = Location_Charging;
    return true; }
    return false;
}
string pathStatusToText(PathStatus status) {
    switch (status) {
    case Path_Open:    
        return "OPEN";
    case Path_Blocked: 
        return "BLOCKED";
    }
    return "UNKNOWN";
}
bool parsePathStatus(string text, PathStatus& outStatus) {
    text = toUpperCase(text);
    if (text == "OPEN") { outStatus = Path_Open;
    return true; }
    if (text == "BLOCKED") { outStatus = Path_Blocked;
    return true; }
    return false;
}
string orderStatusToText(OrderStatus status) {
    switch (status) {
    case Order_Pending:          
        return "PENDING";
    case Order_Reserved:          
        return "RESERVED";
    case Order_Assigned:          
        return "ASSIGNED";
    case Order_PartiallyFulfilled:
        return "PARTIALLY_FULFILLED";
    case Order_Completed:         
        return "COMPLETED";
    case Order_Cancelled:         
        return "CANCELLED";
    }
    return "UNKNOWN";
}
bool parseOrderStatus(string text, OrderStatus& outStatus) {
    text = toUpperCase(text);
    if (text == "PENDING") { 
        outStatus = Order_Pending;           
        return true; }
    if (text == "RESERVED") { 
        outStatus = Order_Reserved;         
        return true; }
    if (text == "ASSIGNED") {
        outStatus = Order_Assigned;          
        return true; }
    if (text == "PARTIALLY_FULFILLED" || text == "PARTIALLYFULFILLED") {
        outStatus = Order_PartiallyFulfilled; 
        return true;
    }
    if (text == "COMPLETED") { 
        outStatus = Order_Completed;   
        return true; }
    if (text == "CANCELLED") { 
        outStatus = Order_Cancelled;     
        return true; }
    return false;
}
string robotStatusToText(RobotStatus status) {
    switch (status) {
    case Robot_Available:          
        return "AVAILABLE";
    case Robot_Assigned:           
        return "ASSIGNED";
    case Robot_WaitingforCharging: 
        return "WAITING_FOR_CHARGING";
    case Robot_Charging:           
        return "CHARGING";
    case Robot_UnAvailable:       
        return "UNAVAILABLE";
    }
    return "UNKNOWN";
}
bool parseRobotStatus(string text, RobotStatus& outStatus) {
    text = toUpperCase(text);
    if (text == "AVAILABLE") {
        outStatus = Robot_Available;          
        return true; }
    if (text == "ASSIGNED") {
        outStatus = Robot_Assigned;        
        return true; }
    if (text == "WAITING_FOR_CHARGING" || text == "WAITINGFORCHARGING") {
        outStatus = Robot_WaitingforCharging; 
        return true;
    }
    if (text == "CHARGING") { 
        outStatus = Robot_Charging;  
        return true; }
    if (text == "UNAVAILABLE" || text == "UN_AVAILABLE") { 
        outStatus = Robot_UnAvailable;   
        return true; }
    return false;
}
string robotTypeToText(RobotType type) {
    switch (type) {
    case Type_Picker: 
        return "PICKER";
    case Type_Carrier:
        return "CARRIER";
    case Type_Heavy:  
        return "HEAVY";
    }
    return "UNKNOWN";
}
bool parseRobotType(string text, RobotType& outType) {
    text = toUpperCase(text);
    if (text == "PICKER") { 
        outType = Type_Picker;  
        return true; }
    if (text == "CARRIER") { 
        outType = Type_Carrier; 
        return true; }
    if (text == "HEAVY") { 
        outType = Type_Heavy;   
        return true; }
    return false;
}
// Battery Formula 
double legConsumption(int distance, double rate, double carriedWeight, double maxCapacity) {
    if (maxCapacity <= 0) 
        return distance * rate; 
    // Safety against division by zero
    return distance * rate * (1.0 + 0.5 * ((double)carriedWeight / maxCapacity));
}
//RouteResult & Linked List
struct RouteNode {
    string locationId;
    RouteNode* next;

    RouteNode(string id) : locationId(id), next(nullptr) {}
};

class RouteResult {
private:
    void copyFrom(const RouteResult& other) {
        head = nullptr;
        totalDistance = other.totalDistance;
        locationCount = other.locationCount;
        found = other.found;

        if (!other.head) 
            return;

        head = new RouteNode(other.head->locationId);
        RouteNode* currTarget = head;
        RouteNode* currSource = other.head->next;

        while (currSource) {
            currTarget->next = new RouteNode(currSource->locationId);
            currTarget = currTarget->next;
            currSource = currSource->next;
        }
    }

public:
    RouteNode* head;
    int totalDistance;
    int locationCount;
    bool found;

    RouteResult() : head(nullptr), totalDistance(0), locationCount(0), found(false) {}

    ~RouteResult() {
        clear();
    }
    RouteResult(const RouteResult& other) {
        copyFrom(other);
    }

    RouteResult& operator=(const RouteResult& other) {
        if (this != &other) {
            clear();
            copyFrom(other);
        }
        return *this;
    }

    void clear() {
        while (head) {
            RouteNode* temp = head;
            head = head->next;
            delete temp;
        }
        totalDistance = 0;
        locationCount = 0;
        found = false;
    }

    void append(string id) {
        RouteNode* newNode = new RouteNode(id);
        if (!head) {
            head = newNode;
        }
        else {
            RouteNode* curr = head;
            while (curr->next) {
                curr = curr->next;
            }
            curr->next = newNode;
        }
        locationCount++;
    }

    string firstId() const {
        return head ? head->locationId : "";
    }

    string lastId() const {
        if (!head) 
            return "";
        RouteNode* curr = head;
        while (curr->next) {
            curr = curr->next;
        }
        return curr->locationId;
    }

    void display() const {
        if (!found) {
            cout << "Route not found.\n";
            return;
        }
        cout << "Route (" << locationCount << " stops, Total Distance: " << totalDistance << "): ";
        RouteNode* curr = head;
        while (curr) {
            cout << curr->locationId << (curr->next ? " -> " : "");
            curr = curr->next;
        }
        cout << endl;
    }
};