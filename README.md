<div align="center"> <!-- Animated GitHub header --> <img src="https://capsule-render.vercel.app/api?type=waving&color=0:111827,50:1d4ed8,100:06b6d4&height=210&section=header&text=AURA&fontSize=76&fontColor=ffffff&animation=fadeIn&fontAlignY=38&desc=Autonomous%20Universal%20Robotic%20Architecture&descAlignY=62&descSize=19" width="100%" alt="AURA animated header" /> <img src="https://readme-typing-svg.demolab.com?font=JetBrains+Mono&weight=700&size=19&duration=2800&pause=700&color=38BDF8&center=true&vCenter=true&width=760&lines=Smart+warehouse+robotics+simulation;AVL+inventory+%7C+Graph+routes+%7C+Robot+fleet;Plan+%E2%86%92+Reserve+%E2%86%92+Pick+%E2%86%92+Deliver;Built+with+C%2B%2B+and+core+data+structures" alt="Animated project description" />


[![Language](https://img.shields.io/badge/language-C%2B%2B20-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](#)
[![Platform](https://img.shields.io/badge/platform-console-111827?style=for-the-badge&logo=windows-terminal&logoColor=white)](#)
[![Data Structures](https://img.shields.io/badge/data%20structures-AVL%20%7C%20Graph%20%7C%20Queue%20%7C%20Stack-7C3AED?style=for-the-badge)](#)
[![Status](https://img.shields.io/badge/status-complete%20simulation-10B981?style=for-the-badge)](#)

**From warehouse rack to customer order — AURA coordinates the entire journey.**

</div>

---

## 🤖 What is AURA?

**AURA — Autonomous Universal Robotic Architecture** is a modular C++ warehouse robotics simulator. It models how a smart warehouse can receive orders, find stock, select the right robot, calculate routes, reserve inventory, execute picking trips, manage battery charging, and complete deliveries.

This is more than a menu-driven program. It is a practical demonstration of how core data structures cooperate inside a real-world system:

<div align="center">

| 🧠 Intelligence | 🗺️ Movement | 📦 Fulfillment | ⚡ Operations |
| --- | --- | --- | --- |
| AVL product search | Graph shortest paths | Order planning | Battery charging |
| Robot selection | BFS connectivity | Stock reservation | Undo history |
| Deterministic tie-breaks | Blocked pathways | Multi-trip execution | Fleet management |

</div>

---

## ✨ The AURA mission

```
┌─────────────────────────────────────────────────────────────────┐
│                         CUSTOMER ORDER                         │
└──────────────────────────────┬──────────────────────────────────┘
                               │
                               ▼
┌──────────────┐      ┌─────────────────┐      ┌─────────────────┐
│ OrderManager │ ───► │ Pending Queue   │ ───► │  PlanManager    │
└──────────────┘      └─────────────────┘      └────────┬────────┘
                                                        │
             ┌──────────────────────────────────────────┼──────────────────────┐
             │                                          │                      │
             ▼                                          ▼                      ▼
     ┌───────────────┐                         ┌──────────────┐        ┌─────────────┐
     │  Inventory    │                         │ Robot Fleet  │        │    Graph    │
     │  AVL + Racks  │                         │ Capacity     │        │ Shortest    │
     └───────┬───────┘                         │ Battery      │        │ Routes      │
             │                                 └──────┬───────┘        └──────┬──────┘
             └──────────────────┬────────────────────┴───────────────────────┘
                                ▼
                    ┌────────────────────────┐
                    │       PickingPlan      │
                    │  Stops + Legs + Trips  │
                    └────────────┬───────────┘
                                 ▼
                    ┌────────────────────────┐
                    │   ExecutionManager     │
                    │  Collect → Pack → Done  │
                    └────────────────────────┘
```

### 🌟 AURA also manages

- 🔋 Charging stations and FIFO charging queues.

- ↩️ Undoable inventory, pathway, and maintenance changes.

- 🧭 Blocked routes and route recalculation.

- 🦾 Picker, Carrier, and Heavy-duty robot classes.

- 🧾 Complete order status progression.

- 🏭 A simple central `WarehouseSystem` coordinator.

---

## 🎯 Feature cards

<table>
<tr>
<td width="50%" valign="top">

### 📦 Intelligent order fulfillment

- Customer orders loaded from files.

- Permanent order history.

- FIFO pending-order queue.

- Multi-product orders.

- Partial fulfillment support.

- Reservation and cancellation rules.

- Automatic completion after the final trip.

</td>
<td width="50%" valign="top">

### 🌳 Fast inventory lookup

- Products stored in an AVL tree.

- Rack stock stored as linked records.

- Stock allocation across reachable racks.

- Reservation lock before execution.

- Collection deduction during execution.

- Low-stock reporting.

</td>
</tr>
<tr>
<td width="50%" valign="top">

### 🗺️ Warehouse route intelligence

- Graph-based warehouse map.

- BFS connectivity checks.

- Dijkstra shortest paths.

- Open and blocked pathways.

- Route validation before execution.

- Deterministic route tie-breaking.

</td>
<td width="50%" valign="top">

### 🦾 Robot fleet control

- Robot inheritance and polymorphism.

- Picker, Carrier, and Heavy-duty types.

- Capacity validation.

- Battery-aware planning.

- Assignment and release states.

- Maintenance mode.

</td>
</tr>
<tr>
<td width="50%" valign="top">

### ⚡ Charging operations

- Low-battery detection.

- Reachable charging-station search.

- Charging-slot allocation.

- FIFO waiting queue.

- Automatic next-robot charging.

- Full battery restoration.

</td>
<td width="50%" valign="top">

### ↩️ Safe undo history

- Linked LIFO undo stack.

- Inventory quantity restoration.

- Pathway reopening.

- Robot maintenance restoration.

- Ownership-safe record cleanup.

- Failed undo actions remain protected.

</td>
</tr>
</table>

---

## 🧩 Data-structure engine

<div align="center">

| Module | Structure | Job |
| --- | --- | --- |
| `ProductAVL` | 🌳 AVL tree | Fast product lookup |
| `RackEntry` | 🔗 Linked list | Stock at multiple racks |
| `OrderManager` | 🔗 Linked list | Permanent order records |
| `OrderQueue` | 🚶 FIFO queue | Pending orders |
| `Graph` | 🕸️ Adjacency lists | Warehouse layout |
| `GraphQueue` | 🚶 Queue | BFS traversal |
| `PlanManager` | 🔗 Linked plan list | Active robot plans |
| `PickingPlan` | 🧱 Linked trips/stops/legs | Physical work plan |
| `ChargingQueue` | 🚶 FIFO queue | Robots waiting to charge |
| `UndoStack` | 📚 LIFO stack | Reversible operations |
| `RobotManager` | 🤖 Linked robot list | Fleet ownership |

</div>

---

## 🦾 Robot intelligence

| Robot | Capacity | Best for | Battery rate |
| --- | --- | --- | --- |
| 🟦 **Picker** | Up to 15 kg | Light products and small orders | 0.20 |
| 🟨 **Carrier** | Up to 30 kg | Medium-weight orders | 0.25 |
| 🟥 **Heavy-duty** | Up to 60 kg | Heavy products and large orders | 0.35 |

Every robot tracks:

```
Robot ID  ·  Current location  ·  Battery  ·  Capacity
Status    ·  Assigned task     ·  Robot type
```

### 🏆 Robot selection rule

AURA compares all feasible plans using a deterministic ranking:

```
🥇 Fewest trips
🥈 Smallest total travel distance
🥉 Smallest robot ID
```

---

## 🔄 Complete execution pipeline

```
📥 Load data
   ↓
🧾 Select pending order
   ↓
📦 Check inventory and remaining quantities
   ↓
🦾 Compare available robots
   ↓
🗺️ Allocate reachable racks and calculate routes
   ↓
🧱 Build trips, stops, and route legs
   ↓
👀 Display plan to user
   ↓
✅ Confirm plan
   ↓
🔒 Reserve stock and assign robot
   ↓
🚚 Execute one trip at a time
   ↓
📦 Collect products and move to packing station
   ↓
📊 Update delivered quantity and order status
   ↓
🏁 Release robot and finish order
```

### 📊 Order lifecycle

```
       ┌─────────┐      ┌──────────┐      ┌──────────┐
       │ Pending │ ───► │ Reserved │ ───► │ Assigned │
       └─────────┘      └──────────┘      └────┬─────┘
                                                │
                             ┌──────────────────┴──────────────────┐
                             ▼                                     ▼
                   ┌────────────────────┐                ┌───────────────┐
                   │ Partially Fulfilled│ ─────────────► │   Completed   │
                   └────────────────────┘                └───────────────┘
```

---

## 🇵🇰 Sample Pakistani warehouse

The included dataset represents a local grocery and household warehouse with products such as:

<div align="center">

| 🍫 | 🥔 | 🥤 | 🌶️ | 🍵 | 🥛 | 🧺 | 🌽 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Dairy Milk | Lays | Rooh Afza | Shan Masala | Tapal Tea | Milkpak | Surf Excel | Kurkure |

</div>

Sample customers and warehouse routes include Lahore, Karachi, Islamabad, and Multan-inspired records.

---

## 🗂️ Project structure

<details>
<summary><b>Click to expand the complete source tree</b></summary>

```
Project1/
│
├── 🚀 main.cpp
├── 🏭 WarehouseSystem.h / WarehouseSystem.cpp
├── ⚙️ Common.h
│
├── 🗺️ Graph.h / Graph.cpp
│
├── 🌳 ProductAVL.h / ProductAVL.cpp
├── 🧱 RackEntry.h / RackEntry.cpp
├── 🗃️ RackRegistry.h / RackRegistry.cpp
├── 📦 InventoryManager.h / InventoryManager.cpp
│
├── 🧾 Order.h / Order.cpp
├── 🚶 OrderQueue.h / OrderQueue.cpp
├── 📋 OrderManager.h / OrderManager.cpp
│
├── 🤖 Robot.h / Robot.cpp
├── 🦾 RobotManager.h / RobotManager.cpp
│
├── 🧭 PickingPlan.h / PickingPlan.cpp
├── 🧠 PlanManager.h / PlanManager.cpp
├── 🚚 ExecutionManager.h / ExecutionManager.cpp
│
├── ⚡ ChargingQueue.h / ChargingQueue.cpp
├── 🔌 ChargingStation.h / ChargingStation.cpp
├── 🔋 ChargingManager.h / ChargingManager.cpp
│
├── ↩️ UndoStack.h / UndoStack.cpp
├── 🛠️ UndoManager.h / UndoManager.cpp
│
└── 📁 Data files
    ├── locations.txt
    ├── pathways.txt
    ├── racks.txt
    ├── products.txt
    ├── inventory.txt
    ├── orders.txt
    ├── robots.txt
    └── charging_stations.txt
```

</details>

---

## 🚀 Quick start

### Visual Studio

1. Open `Project1.slnx` or `Project1.vcxproj`.

1. Set the working directory to the folder containing `main.cpp` and the `.txt` files.

1. Build the project.

1. Run the program.

```
DATA PROJECT/Project1
```

### g++ / MinGW

```bash
g++ -std=c++20 -Wall -Wextra -pedantic \
  *.cpp \
  -o warehouse_app

./warehouse_app
```

> ⚠️ Run the program from the same directory as the data files. AURA loads files using relative paths such as `locations.txt`, `products.txt`, and `orders.txt`.

---

## 🎮 Main menu

```
╔══════════════════════════════════════════════╗
║              PAKISTAN WAREHOUSE SYSTEM       ║
╠══════════════════════════════════════════════╣
║ 1. Warehouse layout and pathways             ║
║ 2. Inventory management                      ║
║ 3. Order management                           ║
║ 4. Robot management                           ║
║ 5. Planning and reservations                  ║
║ 6. Execute assigned orders                    ║
║ 7. Charging management                        ║
║ 8. Undo history                               ║
║ 0. Exit                                       ║
╚══════════════════════════════════════════════╝
```

---


</details>

---

## 🧱 Ownership and design rules

- 🧠 Each manager owns the objects created inside its module.

- 🔗 Managers communicate through references without taking ownership.

- 🚫 Copying is disabled for classes that own linked structures or references.

- 🧹 Destructors clean linked nodes, plans, routes, orders, robots, and queue records.

- 🔤 IDs are normalized to uppercase to prevent duplicates.

- 🔒 Planning validates stock, battery, capacity, and routes before execution.

- 🚶 Charging queues use FIFO behavior.

- 📚 Undo history uses LIFO behavior.

---

## ✅ Project status

<div align="center">

| Area | Status |
| --- | --- |
| Graph loading and shortest routes | ✅ Complete |
| AVL product inventory | ✅ Complete |
| Order queue and status management | ✅ Complete |
| Robot inheritance and fleet management | ✅ Complete |
| Picking-plan generation | ✅ Complete |
| Order execution and collection | ✅ Complete |
| Battery charging and waiting queues | ✅ Complete |
| Undo history | ✅ Complete |
| Central warehouse integration | ✅ Complete |
| Pakistani sample data | ✅ Included |

</div>

---


<div align="center">

### 🌐 AURA turns data structures into warehouse intelligence.

<img src="https://readme-typing-svg.demolab.com?font=JetBrains+Mono&size=15&duration=3000&pause=900&color=22C55E&center=true&vCenter=true&width=650&lines=Plan+smart.+Move+fast.+Deliver+reliably.;Built+for+learning.+Designed+like+a+real+system.;AURA+%E2%80%94+where+code+meets+automation+%F0%9F%A4%96" alt="Animated closing message" />

**AURA** · C++20 · Warehouse Robotics · Data Structures · Automation

</div>
