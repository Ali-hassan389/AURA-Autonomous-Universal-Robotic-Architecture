#include "ExecutionManager.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
using namespace std;


// Turns a number into text with one decimal, used to build reason strings
// Formats diagnostic numeric values consistently for messages.
static string numberToText(double value)
{
	ostringstream stream;
	stream << fixed << setprecision(1) << value;
	return stream.str();
}


//  CONSTRUCTOR 

// Retain references to the managers that own the plan and warehouse state.
ExecutionManager::ExecutionManager(PlanManager& planManager,
	Graph& warehouseGraph,
	InventoryManager& inventoryManager,
	OrderManager& orderManager,
	RobotManager& robotManager)
	: planner(planManager),
	graph(warehouseGraph),
	inventory(inventoryManager),
	orders(orderManager),
	robots(robotManager) {
}


// NEXT TRIP

// Walks the trip list and returns the first trip that is not executed.
// Returns nullptr when every trip is done.
PlanTrip* ExecutionManager::getNextTrip(PickingPlan* plan)
{
	if (plan == nullptr)
		return nullptr;

	PlanTrip* trip = plan->tripHead;

	while (trip != nullptr)
	{
		if (!trip->executed)
			return trip;

		trip = trip->next;
	}

	return nullptr;
}


//  DISPLAY ASSIGNED ORDERS 

// Shows every active plan with its order status and progress.
// Returns the number of plans shown.
// Lists active plans and counts executed trips for each assigned order.
int ExecutionManager::displayAssignedOrders()
{
	PickingPlan* plan = planner.getActiveHead();
	int shown = 0;

	cout << "\n========== ASSIGNED ORDERS ==========\n";

	while (plan != nullptr)
	{
		Order* order = orders.findOrder(plan->orderId);

		int doneTrips = 0;
		PlanTrip* trip = plan->tripHead;

		while (trip != nullptr)
		{
			if (trip->executed)
				doneTrips++;

			trip = trip->next;
		}

		cout << "Order: " << plan->orderId
			<< " | Robot: " << plan->robotId
			<< " | Status: "
			<< (order != nullptr ? orderStatusToText(order->status) : "UNKNOWN")
			<< " | Trips done: " << doneTrips
			<< " of " << plan->tripCount << '\n';

		shown++;
		plan = plan->next;
	}

	if (shown == 0)
		cout << "No assigned orders with a plan.\n";

	return shown;
}


//DISPLAY PLAN FOR EXECUTION 

// Prints the progress of every trip, then the next trip in detail
// Displays robot state, trip progress, and details of the next pending trip.
void ExecutionManager::displayPlanForExecution(PickingPlan* plan)
{
	if (plan == nullptr)
		return;

	Robot* robot = robots.findRobot(plan->robotId);

	cout << "\n========== EXECUTE TASK ==========\n";
	cout << "Order: " << plan->orderId << '\n';
	cout << "Assigned robot: " << plan->robotId;

	if (robot != nullptr)
	{
		cout << " | Location: " << robot->getLocation()
			<< " | Battery: " << fixed << setprecision(1)
			<< robot->getBattery() << "%";
	}

	cout << '\n';

	// One line per trip so the user sees what is done
	PlanTrip* trip = plan->tripHead;

	while (trip != nullptr)
	{
		cout << "Trip " << trip->tripNumber << ": "
			<< (trip->executed ? "EXECUTED" : "PENDING") << '\n';

		trip = trip->next;
	}

	PlanTrip* nextTrip = getNextTrip(plan);

	if (nextTrip == nullptr)
	{
		cout << "All trips are executed.\n";
		return;
	}

	cout << "\n--- Next trip: " << nextTrip->tripNumber << " ---\n";
	cout << "Start location: " << nextTrip->startLocationId << '\n';

	PlanStop* stop = nextTrip->stopHead;

	while (stop != nullptr)
	{
		cout << "Visit rack " << stop->rackId
			<< " (" << stop->rackLocationId << ")"
			<< " - collect " << stop->quantity
			<< " x " << stop->productId << '\n';

		stop = stop->next;
	}

	cout << "Destination (packing station): "
		<< nextTrip->packingStationId << '\n';
	cout << "Total distance: " << nextTrip->totalDistance << '\n';
	cout << "Total weight: " << fixed << setprecision(1)
		<< nextTrip->totalWeight << " kg\n";
	cout << "Battery needed: " << fixed << setprecision(1)
		<< nextTrip->totalBattery << "%\n";
}


// ==================== VALIDATE TRIP ====================

// Read-only checks that run before anything is changed.
// Returns false and fills reason on the first failure.
// Checks trip preconditions without mutating inventory, robot, order, or plan state.
bool ExecutionManager::validateTrip(Order* order, Robot* robot,
	PlanTrip* trip, string& reason)
{
	if (order == nullptr || robot == nullptr || trip == nullptr)
	{
		reason = "Order, robot or trip record is missing.";
		return false;
	}

	// A trip is never executed twice
	if (trip->executed)
	{
		reason = "This trip has already been executed.";
		return false;
	}

	if (robot->getStatus() != Robot_Assigned)
	{
		reason = "Robot " + robot->getId() + " is not in the Assigned state.";
		return false;
	}

	// Check 1: every leg of the trip must still use open pathways
	RouteLeg* leg = trip->legHead;

	while (leg != nullptr)
	{
		if (!graph.isRouteStillOpen(leg->route))
		{
			reason = "Route " + leg->fromId + " -> " + leg->toId
				+ " is blocked or no longer exists. "
				+ "Use Recalculate Route.";
			return false;
		}

		leg = leg->next;
	}

	// Check 2: robot must be standing at the start of the trip
	if (toUpperCase(robot->getLocation()) !=
		toUpperCase(trip->startLocationId))
	{
		reason = "Robot is at " + robot->getLocation()
			+ " but the trip starts at " + trip->startLocationId + ".";
		return false;
	}

	// Check 3: enough battery for the trip plus the safety reserve
	double neededBattery = trip->totalBattery + Safety_Reserve;

	if (robot->getBattery() < neededBattery)
	{
		reason = "Battery too low. Needs " + numberToText(neededBattery)
			+ "% (trip + reserve) but robot has "
			+ numberToText(robot->getBattery())
			+ "%. Suggestion: charge robot " + robot->getId()
			+ " (Module 7).";
		return false;
	}

	// Check 4: trip weight must fit the robot
	if (trip->totalWeight > robot->getCapacity())
	{
		reason = "Trip weight " + numberToText(trip->totalWeight)
			+ " kg is above the robot capacity of "
			+ numberToText(robot->getCapacity()) + " kg.";
		return false;
	}

	// Check 5: every stop is not collected yet and stock can be collected
	PlanStop* stop = trip->stopHead;

	while (stop != nullptr)
	{
		if (stop->collected)
		{
			reason = "Stop at rack " + stop->rackId
				+ " is already collected.";
			return false;
		}

		if (!inventory.canCollect(order->orderId, stop->productId,
			stop->rackId, stop->quantity))
		{
			reason = "Cannot collect " + stop->productId
				+ " from rack " + stop->rackId
				+ ": reserved stock is not available.";
			return false;
		}

		stop = stop->next;
	}

	return true;
}


// ==================== EXECUTE TRIP ====================

// Validates the next trip, then applies it step by step
// Executes the first pending trip for an order after validation succeeds.
bool ExecutionManager::executeTrip(const string& orderId)
{
	string cleanOrderId = toUpperCase(trimText(orderId));

	PickingPlan* plan = planner.findPlan(cleanOrderId);
	Order* order = orders.findOrder(cleanOrderId);

	if (plan == nullptr || order == nullptr)
	{
		cout << "No active plan found for this order.\n";
		return false;
	}

	Robot* robot = robots.findRobot(plan->robotId);

	if (robot == nullptr)
	{
		cout << "Trip rejected: assigned robot no longer exists.\n";
		return false;
	}

	PlanTrip* trip = getNextTrip(plan);

	if (trip == nullptr)
	{
		cout << "Every trip of this order is already executed.\n";
		return false;
	}

	// Validate first, nothing is changed if a check fails
	string reason;

	if (!validateTrip(order, robot, trip, reason))
	{
		cout << "Trip rejected: " << reason << '\n';
		cout << "No records were changed.\n";
		return false;
	}

	// ----- Apply steps, only reached when every check passed -----

	// Step 1: collect every stop (rack total and reserved drop together)
	PlanStop* stop = trip->stopHead;

	while (stop != nullptr)
	{
		if (!inventory.collect(order->orderId, stop->productId,
			stop->rackId, stop->quantity))
		{
			// canCollect passed, so this should never happen
			cout << "Error: collection failed at rack "
				<< stop->rackId << ". Stopping this trip.\n";
			return false;
		}

		stop->collected = true;
		stop = stop->next;
	}

	// Step 2: robot moves to the packing station and loses battery
	double newBattery = robot->getBattery() - trip->totalBattery;

	if (newBattery < 0)
		newBattery = 0;

	robot->setLocation(trip->packingStationId);
	robot->setBattery(newBattery);

	// Step 3: add every stop quantity to the matching order item
	stop = trip->stopHead;

	while (stop != nullptr)
	{
		OrderItem* item = order->findItem(stop->productId);

		if (item != nullptr)
			item->deliveredQty += stop->quantity;

		stop = stop->next;
	}

	// Step 4: mark the trip as executed
	trip->executed = true;

	// Step 5: Completed only when every item is fully delivered.
	// Status is changed only when it differs, because Partially
	// Fulfilled to Partially Fulfilled is not a legal move.
	OrderStatus newStatus = order->isFullyDelivered()
		? Order_Completed
		: Order_PartiallyFulfilled;

	if (order->status != newStatus)
		orders.updateStatus(cleanOrderId, newStatus);

	// Step 7: delivery summary (printed before the plan can be deleted)
	cout << "\n========== DELIVERY SUMMARY ==========\n";
	cout << "Trip " << trip->tripNumber << " executed for order "
		<< cleanOrderId << ".\n";
	cout << "Delivered to packing station: "
		<< trip->packingStationId << '\n';
	cout << "Robot " << robot->getId() << " new location: "
		<< robot->getLocation() << '\n';
	cout << "Robot " << robot->getId() << " new battery: "
		<< fixed << setprecision(1) << robot->getBattery() << "%\n";
	cout << "Order status: " << orderStatusToText(order->status) << '\n';

	// Step 6: after the last trip free the robot and discard the plan
	if (getNextTrip(plan) == nullptr)
		finishOrderIfDone(cleanOrderId);

	return true;
}


// ==================== FINISH ORDER ====================

// Runs after the last trip: releases the robot and removes the plan.
// The final order status was already set by executeTrip.
void ExecutionManager::finishOrderIfDone(const string& orderId)
{
	string cleanOrderId = toUpperCase(trimText(orderId));

	PickingPlan* plan = planner.findPlan(cleanOrderId);
	Order* order = orders.findOrder(cleanOrderId);

	if (plan == nullptr || order == nullptr)
		return;

	// Only finish when no trip is left
	if (getNextTrip(plan) != nullptr)
		return;

	robots.releaseRobot(plan->robotId);
	order->assignedRobotId = "";

	planner.discardPlan(cleanOrderId);

	cout << "All trips finished. Robot released and plan discarded.\n";
	cout << "Final order status: "
		<< orderStatusToText(order->status) << '\n';
}


// ==================== RECALCULATE ROUTE ====================

// Re-runs Dijkstra between the same stops for every unexecuted trip.
// All legs are checked first, so the plan is only changed when every
// leg still has a route.
// Recomputes routes for remaining trips and updates their distance/battery totals.
bool ExecutionManager::recalculateRoute(const string& orderId)
{
	string cleanOrderId = toUpperCase(trimText(orderId));

	PickingPlan* plan = planner.findPlan(cleanOrderId);
	Robot* robot = nullptr;

	if (plan == nullptr)
	{
		cout << "No active plan found for this order.\n";
		return false;
	}

	robot = robots.findRobot(plan->robotId);

	if (robot == nullptr)
	{
		cout << "Assigned robot no longer exists.\n";
		return false;
	}

	// Pass 1: make sure every leg still has a route, change nothing
	PlanTrip* trip = plan->tripHead;

	while (trip != nullptr)
	{
		if (!trip->executed)
		{
			RouteLeg* leg = trip->legHead;

			while (leg != nullptr)
			{
				RouteResult newRoute =
					graph.shortestRoute(leg->fromId, leg->toId);

				if (!newRoute.found)
				{
					// Find the rack that is waiting at the unreachable place
					string rackText = "";
					PlanStop* stop = trip->stopHead;

					while (stop != nullptr)
					{
						if (stop->rackLocationId == leg->toId)
						{
							rackText = " (rack " + stop->rackId + ")";
							break;
						}

						stop = stop->next;
					}

					cout << "No route from " << leg->fromId
						<< " to " << leg->toId << rackText
						<< ". That location is unreachable.\n";
					cout << "The plan was not changed. "
						<< "Use Cancel Assignment to replan.\n";

					return false;
				}

				leg = leg->next;
			}
		}

		trip = trip->next;
	}

	// Pass 2: every route exists, so rebuild legs and totals
	trip = plan->tripHead;

	while (trip != nullptr)
	{
		if (!trip->executed)
		{
			RouteLeg* leg = trip->legHead;

			trip->totalDistance = 0;
			trip->totalBattery = 0.0;

			while (leg != nullptr)
			{
				RouteResult newRoute =
					graph.shortestRoute(leg->fromId, leg->toId);

				leg->route = newRoute;
				leg->distance = newRoute.totalDistance;
				leg->battery = robot->consumption(
					newRoute.totalDistance, leg->carriedWeight);

				trip->totalDistance += leg->distance;
				trip->totalBattery += leg->battery;

				leg = leg->next;
			}
		}

		trip = trip->next;
	}

	// Update the plan totals from all trips (executed ones keep their values)
	plan->totalDistance = 0;
	plan->totalBattery = 0.0;

	trip = plan->tripHead;

	while (trip != nullptr)
	{
		plan->totalDistance += trip->totalDistance;
		plan->totalBattery += trip->totalBattery;
		trip = trip->next;
	}

	cout << "Routes recalculated successfully.\n";
	cout << "New plan distance: " << plan->totalDistance
		<< " | New plan battery: " << fixed << setprecision(1)
		<< plan->totalBattery << "%\n";

	return true;
}


// ==================== CANCEL ASSIGNMENT ====================

// Undoes the assignment so the order can be planned again.
// Already delivered items stay delivered.
// Releases an active assignment and returns the order to the pending queue.
bool ExecutionManager::cancelAssignment(const string& orderId)
{
	string cleanOrderId = toUpperCase(trimText(orderId));

	PickingPlan* plan = planner.findPlan(cleanOrderId);
	Order* order = orders.findOrder(cleanOrderId);

	if (plan == nullptr || order == nullptr)
	{
		cout << "No active plan found for this order.\n";
		return false;
	}

	if (order->status != Order_Assigned &&
		order->status != Order_PartiallyFulfilled)
	{
		cout << "This order has no active assignment to cancel.\n";
		return false;
	}

	// Step 1: ask for confirmation
	if (!readYesNo("Cancel the assignment and send the order back to the queue?"))
	{
		cout << "Cancellation aborted. Nothing was changed.\n";
		return false;
	}

	// The robot ID must be saved before the plan is deleted
	string robotId = plan->robotId;

	// Step 2: release reservations that were not collected yet
	inventory.releaseAllReservations(cleanOrderId);

	// Step 3: free the robot and clear the order's robot
	robots.releaseRobot(robotId);
	order->assignedRobotId = "";

	// Step 4: discard the plan (the plan pointer is invalid after this)
	planner.discardPlan(cleanOrderId);

	// Step 5: back to Pending and to the front of the queue
	orders.updateStatus(cleanOrderId, Order_Pending);
	orders.getQueue().pushFront(order);

	cout << "Assignment cancelled. Order " << cleanOrderId
		<< " is Pending again at the front of the queue.\n";

	return true;
}


// ==================== EXECUTE TASK MENU ====================

// Screen for one order. Loops until the user leaves or the plan is gone.
// Interactive loop for executing, rerouting, or cancelling one assigned order.
void ExecutionManager::executeTaskMenu(const string& orderId)
{
	string cleanOrderId = toUpperCase(trimText(orderId));

	while (true)
	{
		PickingPlan* plan = planner.findPlan(cleanOrderId);

		if (plan == nullptr)
		{
			cout << "No active plan remains for order "
				<< cleanOrderId << ".\n";
			return;
		}

		displayPlanForExecution(plan);

		cout << "\n1. Execute Trip\n";
		cout << "2. Recalculate Route\n";
		cout << "3. Cancel Assignment\n";
		cout << "4. Assign but do not execute now\n";

		int choice = readIntInRange("Enter choice: ", 1, 4);

		if (choice == 1)
		{
			executeTrip(cleanOrderId);
		}
		else if (choice == 2)
		{
			recalculateRoute(cleanOrderId);
		}
		else if (choice == 3)
		{
			cancelAssignment(cleanOrderId);
		}
		else
		{
			cout << "Order stays assigned. You can execute it later.\n";
			return;
		}

		pauseScreen();
	}
}


// ==================== EXECUTION MENU ====================

// Top-level menu: select an assigned order, or return when none remain.
void runExecutionMenu(ExecutionManager& executor)
{
	while (true)
	{
		int count = executor.displayAssignedOrders();

		if (count == 0)
			return;

		string orderId = readNonEmptyString(
			"Enter order ID to execute (0 to go back): ");

		if (orderId == "0")
			return;

		executor.executeTaskMenu(orderId);
	}
}
