# Optimal Route Management System

## Project Overview

The Optimal Route Management System is a C++-based graph optimization project designed to find efficient routes in weighted road networks under dynamic traffic conditions.

The project implements multiple pathfinding algorithms including A* Search, Dijkstra's Algorithm, and Breadth First Search (BFS) to compute routes between source and destination nodes. It also incorporates Dynamic Traffic Simulation, Route Exclusion Management using Red-Black Trees, Dynamic Route Recalculation, and a Benchmarking Framework for performance evaluation.

The primary objective of this project is to simulate real-world route optimization scenarios where road conditions may change over time and users may wish to avoid specific routes while still obtaining an optimal path.

---

# Key Features

* A* Search Algorithm
* Dijkstra's Algorithm
* Breadth First Search (BFS)
* Binary Min Heap Priority Queue
* Red-Black Tree based Route Exclusion System
* Dynamic Traffic Simulation
* Dynamic Route Recalculation
* CSV-based Graph Loading
* Grid Graph Generation
* Route Analytics Dashboard
* Performance Benchmarking Framework
* Execution Time Analysis
* Memory Usage Analysis
* Nodes Explored Analysis

---

# System Components

## 1. A* Search Engine

The A* Search algorithm is the primary route-finding algorithm used in this project.

Features:

* Heuristic-guided shortest path search
* Reduced search space compared to Dijkstra
* Supports weighted graphs
* Produces optimal paths when the heuristic is admissible

The evaluation function is:

f(n) = g(n) + h(n)

Where:

* g(n) = Actual cost from source to current node
* h(n) = Estimated cost from current node to destination
* f(n) = Total estimated path cost

The implementation uses Euclidean Distance as the heuristic function.

---

## 2. Dijkstra's Algorithm

Dijkstra's Algorithm computes shortest paths from a source node to all reachable nodes in a weighted graph.

Characteristics:

* Guarantees optimal shortest path
* Does not require a heuristic
* Uses a Binary Min Heap for efficient node selection
* Serves as a baseline for comparison with A*

Time Complexity:

O((V + E) log V)

Where:

* V = Number of vertices
* E = Number of edges

---

## 3. Breadth First Search (BFS)

BFS is included for graph traversal and benchmarking purposes.

Characteristics:

* Traverses nodes level by level
* Suitable for unweighted shortest path problems
* Provides a comparison against weighted shortest-path algorithms

Time Complexity:

O(V + E)

---

## 4. Binary Min Heap

The Binary Min Heap is used as the priority queue implementation for both A* and Dijkstra.

Supported Operations:

* Insert
* Extract Minimum
* Decrease Key

Complexities:

| Operation    | Complexity |
| ------------ | ---------- |
| Insert       | O(log V)   |
| Extract Min  | O(log V)   |
| Decrease Key | O(log V)   |

The Binary Heap significantly improves performance compared to a linear priority queue.

---

## 5. Red-Black Tree Route Exclusion System

The project uses a Red-Black Tree to manage avoided routes.

Users can:

* Add avoided routes
* Remove avoided routes
* View active route exclusions

Before expanding any edge, the pathfinding algorithms check whether the edge exists in the Red-Black Tree.

If the edge is excluded, it is skipped during route computation.

Advantages:

* O(log n) insertion
* O(log n) deletion
* O(log n) lookup

This makes route exclusion management efficient even for large graphs.

---

# Dynamic Traffic Simulation

To simulate real-world traffic conditions, edge weights can be modified dynamically.

Supported Modes:

### Normal Traffic

Original edge weights are used.

### Peak Hour Traffic

Moderate increase in travel costs.

### Heavy Traffic

Significant increase in travel costs.

Traffic updates affect route costs and may trigger route recalculation.

---

# Dynamic Route Recalculation

One of the core features of this project is Dynamic Route Recalculation.

Whenever:

* Traffic conditions change
* A route is excluded

the system automatically recomputes the optimal path.

The dashboard displays:

* Previous Route
* Updated Route
* Previous Cost
* Updated Cost
* Cost Difference

This simulates adaptive navigation systems used in modern GPS applications.

---

# Route Exclusion System

Users can exclude specific roads from route computation.

Example:

Avoid Route:

13 -> 37

The excluded route is stored in the Red-Black Tree and ignored by:

* A*
* Dijkstra
* BFS

This allows users to customize route preferences.

---

# Graph Representation

The project uses an Adjacency List representation.

Advantages:

* Memory efficient
* Suitable for sparse graphs
* Faster traversal compared to adjacency matrices

Space Complexity:

O(V + E)

---

# CSV Input Format

Graphs can be loaded from CSV files.

Format:

source,destination,weight

Example:

0,1,9
0,2,55
1,4,28
4,10,53
10,22,34

Where:

* source = Starting node
* destination = Ending node
* weight = Cost of traversal

The CSV loader validates:

* File existence
* Node IDs
* Edge weights
* Invalid rows

---

# Benchmark Framework

The project includes a benchmarking system that compares:

* BFS
* Dijkstra
* A*

Metrics Collected:

* Path Cost
* Nodes Explored
* Execution Time
* Memory Usage
* Routes Skipped

The benchmark framework executes multiple source-destination queries and computes average performance metrics.

---

# Performance Analytics

The analytics module automatically computes:

### A* Speedup Over Dijkstra

Measures the reduction in execution time achieved by A*.

### Search Space Reduction

Measures the reduction in explored nodes.

### Memory Usage Comparison

Compares memory requirements across algorithms.

Example Output:

* A* explored 42% fewer nodes than Dijkstra.
* A* achieved 1.8× faster execution time.

---

# Complexity Analysis

| Operation         | Complexity       |
| ----------------- | ---------------- |
| BFS               | O(V + E)         |
| Dijkstra          | O((V + E) log V) |
| A* Search         | O((V + E) log V) |
| Heap Insert       | O(log V)         |
| Heap Extract Min  | O(log V)         |
| Heap Decrease Key | O(log V)         |
| RB Tree Search    | O(log N)         |
| RB Tree Insert    | O(log N)         |
| RB Tree Delete    | O(log N)         |

---

# How to Compile

Using GCC:

```bash
g++ -std=c++17 main.cpp -o route_system
```

---

# How to Run

```bash
./route_system
```

---

# Sample Workflow

1. Load graph from CSV.
2. Select traffic mode.
3. Find shortest path.
4. Add avoided routes.
5. Observe route recalculation.
6. Run benchmarks.
7. Compare algorithm performance.

---

# Technologies Used

* C++
* Graph Algorithms
* Data Structures
* Binary Heaps
* Red-Black Trees
* CSV Processing
* Benchmarking
* Performance Analysis

---

# Future Improvements

Potential future extensions include:

* Real-world road network datasets
* Live traffic API integration
* GUI-based route visualization
* Web application deployment
* GPS integration
* Large-scale city routing

---

# Resume Highlights

* Developed a route optimization system using A*, Dijkstra, and BFS on weighted graph networks.
* Optimized shortest-path computation using a Binary Min Heap priority queue.
* Implemented a Red-Black Tree based route exclusion mechanism with O(log n) operations.
* Built a benchmarking framework comparing pathfinding algorithms using execution time, memory usage, path cost, and nodes explored.
* Added dynamic traffic simulation and automatic route recalculation to adapt routes under changing network conditions.
* Integrated CSV-based graph loading and large-scale graph testing for performance evaluation.

---

# Author

Gowtham

