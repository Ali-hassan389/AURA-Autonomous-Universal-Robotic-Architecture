#include"RackEntry.h"


// Constructor to initialize values
ReservationRecord::ReservationRecord(string o, int q) {
	orderId = o;
	quantity = q;
	NextRecord = NULL;
}
// private helper functions to find record by ID
ReservationRecord* RackEntry::findRecord(string orderId) {
	ReservationRecord* current = reservationHead;
	while (current != NULL) {
		if (current->orderId == orderId) {
			return current;
		}
		current = current->NextRecord;
	}
	return NULL;
}

// Constructor to initilaize
RackEntry::RackEntry(const string rackId, int totalQty) {
	reservedQty = 0;
	reservationHead = NULL;
	tempTaken = 0;
	nextRack = NULL;
	this->totalQty = totalQty;
	this->rackId = rackId;
}
// Destructor to delete Records
RackEntry::~RackEntry() {
	ReservationRecord* current = reservationHead;
	while (current != NULL) {
		ReservationRecord* next = current->NextRecord;
		delete current;
		current = next;
	}
}
// Getter functions to access private members
string RackEntry:: getRackId() {
	return rackId;
}
int RackEntry::getTotalQty() {
	return totalQty;
}
int RackEntry::getReservedQty() {
	return reservedQty;
}
int RackEntry::availableQty() {
	return totalQty - reservedQty;
}
// if already record present , just increase its quantity else add it to begining
bool RackEntry::addReservation(string orderId, int qty) {
	if (qty <= 0) {
		return false;
	}
	if (qty > availableQty()) {
		return false;
	}
	// we find if the record to add already exists if yes then we increase its quantity
	ReservationRecord* Record = findRecord(orderId);
	if (Record != nullptr) {
		Record->quantity += qty;
	}
	// if not found means it doesnt exist so we add it at beggining
	else {
		ReservationRecord* newRecord = new ReservationRecord(orderId, qty);
		newRecord->NextRecord = reservationHead;
		reservationHead = newRecord;
	}
	reservedQty += qty;
	return true;
}

// deletes a reservation record from the linked list and minus from reserved quantity
int RackEntry::releaseReservation(string id) {
	ReservationRecord* current = reservationHead;
	ReservationRecord* previous = nullptr;
	while (current != nullptr) {
		if (current->orderId == id) {
			int releasedQty = current->quantity;
			reservedQty -= releasedQty;
			if (previous == nullptr) {
				reservationHead = current->NextRecord;
			}
			else {
				previous->NextRecord = current->NextRecord;
			}

			delete current;
			return releasedQty;
		}
		previous = current;
		current = current->NextRecord;
	}

	return 0;
}

// check and return quantity reserved for an order
int RackEntry::reservedFor(string orderId) {
	 ReservationRecord* record = findRecord(orderId);
	if (record != nullptr) {
		return record->quantity;
	}
	return 0;
}

// this is just a validation function
bool RackEntry:: canConsume(string orderId, int qty) {
	if (qty <= 0)
		return false;
	if (totalQty < qty)
		return false;
	if (reservedFor(orderId) < qty)
		return false;
	return true;
}

// Consume For Collection
bool RackEntry::consumeForCollection(string orderId, int qty) {
	if (!canConsume(orderId, qty)) {
		return false;
	}

	ReservationRecord* current = reservationHead;
	ReservationRecord* previous = nullptr;

	while (current != nullptr) {
		if (current->orderId == orderId) {
			current->quantity -= qty;
			// If the record reaches 0,we delete it
			if (current->quantity == 0) {
				if (previous == nullptr) {
					reservationHead = current->NextRecord;
				}
				else {
					previous->NextRecord = current->NextRecord;
				}
				delete current;
			}
			break;
		}
		previous = current;
		current = current->NextRecord;
	}

	// Both quantities will decrease here
	reservedQty -= qty;
	totalQty -= qty;
	return true;
}

bool RackEntry::setTotalQty(int newTotal) {
	if (newTotal < 0) {
		return false;
	}
	if (newTotal < reservedQty) {
		return false;
	}
	totalQty = newTotal;
	return true;
}

// Can Be Removed
bool RackEntry::canBeRemoved() {
	return (totalQty == 0 && reservedQty == 0);
}

// Check Invariant
bool RackEntry::checkInvariant() {
	int sum = 0;
	ReservationRecord* current = reservationHead;
	while (current != nullptr) {
		sum += current->quantity;
		current = current->NextRecord;
	}
	return (sum == reservedQty);
}

// Display Row
void RackEntry::display() {
	cout << "Rack: " << rackId << " | Total: " << totalQty << " | Reserved: " << reservedQty
		<< " | Available: " << availableQty() << endl;
}