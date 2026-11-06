#pragma once

#include<iostream>
#include<string>
#include "Common.h"
using namespace std;


// Reservation Record Node
struct ReservationRecord {
	string orderId;
	int quantity;
	ReservationRecord* NextRecord;
	ReservationRecord(string o, int q); 
};

// Rack Entry Linked List
class RackEntry {
	string rackId;
	int totalQty;
	int reservedQty;
	ReservationRecord* reservationHead;

	// private helper functions to find record by ID
	ReservationRecord* findRecord(string orderId);
	
public:
	int tempTaken;
	RackEntry* nextRack;
	
	// Constructor to initilaize
	RackEntry(const string rackId, int totalQty); 
	// Destructor to delete Records
	~RackEntry(); 

	// Getter functions to access private members
	string getRackId(); 
	int getTotalQty();
	int getReservedQty(); 
	int availableQty(); 

	// if already record present , just increase its quantity else add it to begining
	bool addReservation(string orderId, int qty); 

	// deletes a reservation record from the linked list and minus from reserved quantity
	int releaseReservation(string id);

	// check and return quantiy reserved for an order
	int reservedFor(string orderId);

	// this is just a validation function
	bool canConsume(string orderId, int qty);

	// Consume For Collection
	bool consumeForCollection(string orderId, int qty);

	bool setTotalQty(int newTotal);

	// Can Be Removed check 
	bool canBeRemoved();

	// Check Invariant
	bool checkInvariant();

	// Display Row
	void display();

};

