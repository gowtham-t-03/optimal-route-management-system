#include <iostream>
#include <vector>
#include <cmath>
#include <climits>
#include <ctime>
#include <cstdlib>
#include <string>
#include <fstream>
#include <iomanip>
#include <queue>
#include <sstream>
#include <algorithm>

using namespace std;

const int INF = INT_MAX;

enum TrafficMode { TRAFFIC_NORMAL, TRAFFIC_PEAK_HOUR, TRAFFIC_HEAVY };
enum RBTColor { RED, BLACK };

struct PerformanceMetrics {
    long long nodes_explored = 0;
    long long routes_skipped = 0;
    double execution_time_ms = 0.0;
    size_t memory_used_bytes = 0;
    int path_cost = INF;
    int hop_count = 0;
};

struct Coordinate {
    int x;
    int y;
};

struct HeapNode {
    int vertex;
    int key; 
    int g_cost;
};

// MIN- Heap data structure 
class MinHeap {
private:
    vector<HeapNode> nodes;
    vector<int> pos; 
    int heap_size;

    void bubbleUp(int i) {
        while (i > 0 && nodes[i].key < nodes[(i - 1) / 2].key) {
            pos[nodes[i].vertex] = (i - 1) / 2;
            pos[nodes[(i - 1) / 2].vertex] = i;
            swap(nodes[i], nodes[(i - 1) / 2]);
            i = (i - 1) / 2;
        }
    }

    void minHeapify(int idx) {
        int smallest = idx;
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;

        if (left < heap_size && nodes[left].key < nodes[smallest].key)
            smallest = left;
        if (right < heap_size && nodes[right].key < nodes[smallest].key)
            smallest = right;

        if (smallest != idx) {
            pos[nodes[smallest].vertex] = idx;
            pos[nodes[idx].vertex] = smallest;
            swap(nodes[smallest], nodes[idx]);
            minHeapify(smallest);
        }
    }

public:
    MinHeap(int cap) : heap_size(0) {
        nodes.resize(cap);
        pos.assign(cap, -1);
    }

    bool empty() const { return heap_size == 0; }

    void insert(int vertex, int key, int g_cost) {
        if (vertex >= (int)pos.size()) {
            pos.resize(vertex + 1, -1);
        }
        if (heap_size >= (int)nodes.size()) {
            nodes.push_back({vertex, key, g_cost});
        } else {
            nodes[heap_size] = {vertex, key, g_cost};
        }
        heap_size++;
        int i = heap_size - 1;
        pos[vertex] = i;
        bubbleUp(i);
    }

    HeapNode extractMin() {
        if (empty()) return {-1, INF, INF};
        HeapNode rootNode = nodes[0];
        pos[rootNode.vertex] = -1;
        
        if (heap_size > 1) {
            HeapNode lastNode = nodes[heap_size - 1];
            nodes[0] = lastNode;
            pos[lastNode.vertex] = 0;
        }
        heap_size--;
        if (heap_size > 0) {
            minHeapify(0);
        }
        return rootNode;
    }

    void decreaseKey(int vertex, int new_key, int new_g_cost) {
        if (vertex >= (int)pos.size() || pos[vertex] == -1) {
            insert(vertex, new_key, new_g_cost);
            return;
        }
        int i = pos[vertex];
        if (new_key >= nodes[i].key) return;
        nodes[i].key = new_key;
        nodes[i].g_cost = new_g_cost;
        bubbleUp(i);
    }
};

// 2. TEXTBOOK RED-BLACK TREE FOR ROUTE EXCLUSIONS
struct RBTNode {
    long long key; 
    int u, v;
    RBTColor color;
    RBTNode *left, *right, *parent;
    RBTNode(int src, int dst, RBTColor c = RED) : u(src), v(dst), color(c), left(nullptr), right(nullptr), parent(nullptr) {
        key = (static_cast<long long>(src) << 32) | (static_cast<long long>(dst) & 0xFFFFFFFFLL);
    }
};

class RedBlackTree {
private:
    RBTNode* NIL;
    RBTNode* root;
    int count;

    void leftRotate(RBTNode* x) {
        RBTNode* y = x->right;
        x->right = y->left;
        if (y->left != NIL) y->left->parent = x;
        y->parent = x->parent;
        if (x->parent == NIL) root = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;
        y->left = x;
        x->parent = y;
    }

    void rightRotate(RBTNode* x) {
        RBTNode* y = x->left;
        x->left = y->right;
        if (y->right != NIL) y->right->parent = x;
        y->parent = x->parent;
        if (x->parent == NIL) root = y;
        else if (x == x->parent->right) x->parent->right = y;
        else x->parent->left = y;
        y->right = x;
        x->parent = y;
    }

    void fixInsert(RBTNode* z) {
        while (z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                RBTNode* y = z->parent->parent->right;
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        leftRotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rightRotate(z->parent->parent);
                }
            } else {
                RBTNode* y = z->parent->parent->left;
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rightRotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    leftRotate(z->parent->parent);
                }
            }
        }
        root->color = BLACK;
    }

    void rbTransplant(RBTNode* u, RBTNode* v) {
        if (u->parent == NIL) root = v;
        else if (u == u->parent->left) u->parent->left = v;
        else u->parent->right = v;
        v->parent = u->parent;
    }

    void fixDelete(RBTNode* x) {
        while (x != root && x->color == BLACK) {
            if (x == x->parent->left) {
                RBTNode* w = x->parent->right;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    leftRotate(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == BLACK && w->right->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->right->color == BLACK) {
                        w->left->color = BLACK;
                        w->color = RED;
                        rightRotate(w);
                        w = x->parent->right;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->right->color = BLACK;
                    leftRotate(x->parent);
                    x = root;
                }
            } else {
                RBTNode* w = x->parent->left;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    rightRotate(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == BLACK && w->left->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->left->color == BLACK) {
                        w->right->color = BLACK;
                        w->color = RED;
                        leftRotate(w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->left->color = BLACK;
                    rightRotate(x->parent);
                    x = root;
                }
            }
        }
        x->color = BLACK;
    }

    void clearTreeMemory(RBTNode* node) {
        if (node == NIL || node == nullptr) return;
        clearTreeMemory(node->left);
        clearTreeMemory(node->right);
        delete node;
    }

    void displayInOrder(RBTNode* node) const {
        if (node == NIL || node == nullptr) return;
        displayInOrder(node->left);
        cout << "  • [" << node->u << " -> " << node->v << "]\n";
        displayInOrder(node->right);
    }

public:
    RedBlackTree() {
        NIL = new RBTNode(-1, -1, BLACK);
        NIL->left = NIL->right = NIL->parent = NIL;
        root = NIL;
        count = 0;
    }

    ~RedBlackTree() {
        clearTreeMemory(root);
        delete NIL;
    }

    int getCount() const { return count; }

    bool rbt_search(int u, int v) const {
        long long search_key = (static_cast<long long>(u) << 32) | (static_cast<long long>(v) & 0xFFFFFFFFLL);
        RBTNode* curr = root;
        while (curr != NIL) {
            if (search_key == curr->key) return true;
            if (search_key < curr->key) curr = curr->left;
            else curr = curr->right;
        }
        return false;
    }

    void rbt_insert(int u, int v) {
        if (rbt_search(u, v)) return;
        RBTNode* z = new RBTNode(u, v, RED);
        z->left = NIL;
        z->right = NIL;
        z->parent = NIL;

        RBTNode* y = NIL;
        RBTNode* x = root;

        while (x != NIL) {
            y = x;
            if (z->key < x->key) x = x->left;
            else x = x->right;
        }

        z->parent = y;
        if (y == NIL) root = z;
        else if (z->key < y->key) y->left = z;
        else y->right = z;

        fixInsert(z);
        count++;
    }

    void rbt_delete(int u, int v) {
        long long search_key = (static_cast<long long>(u) << 32) | (static_cast<long long>(v) & 0xFFFFFFFFLL);
        RBTNode* z = root;
        while (z != NIL) {
            if (search_key == z->key) break;
            if (search_key < z->key) z = z->left;
            else z = z->right;
        }
        if (z == NIL) return; 

        RBTNode* x;
        RBTNode* y = z;
        RBTColor y_original_color = y->color;

        if (z->left == NIL) {
            x = z->right;
            rbTransplant(z, z->right);
        } else if (z->right == NIL) {
            x = z->left;
            rbTransplant(z, z->left);
        } else {
            y = z->right;
            while (y->left != NIL) {
                y = y->left;
            }
            y_original_color = y->color;
            x = y->right;
            if (y->parent == z) {
                x->parent = y;
            } else {
                rbTransplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            rbTransplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }
        delete z;
        if (y_original_color == BLACK) {
            fixDelete(x);
        }
        count--;
    }

    void listExclusions() const {
        if (count == 0) {
            cout << "  No active route exclusions registered.\n";
            return;
        }
        displayInOrder(root);
    }
};

RedBlackTree globalExclusionTree;

// 3. GRAPH REPRESENTATION ENGINE
struct AdjNode {
    int vertex;
    int base_weight;
    int current_weight;
    AdjNode* next;
    AdjNode(int v, int w) : vertex(v), base_weight(w), current_weight(w), next(nullptr) {}
};

struct AdjList {
    AdjNode* head = nullptr;
};

class Graph {
public:
    int num_vertices;
    string type_label;
    vector<AdjList> array;
    vector<Coordinate> locations;

    Graph(int vertices, string label = "Grid") : num_vertices(vertices), type_label(label) {
        array.resize(vertices);
        locations.resize(vertices);
        for (int i = 0; i < vertices; i++) {
            locations[i].x = (i % 100) * 10;
            locations[i].y = (i / 100) * 10;
        }
    }

    ~Graph() {
        for (int i = 0; i < num_vertices; i++) {
            AdjNode* curr = array[i].head;
            while (curr) {
                AdjNode* temp = curr;
                curr = curr->next;
                delete temp;
            }
        }
    }

    void addEdge(int u, int v, int weight) {
        if (u >= num_vertices || v >= num_vertices || u < 0 || v < 0) return;
        AdjNode* node = new AdjNode(v, weight);
        node->next = array[u].head;
        array[u].head = node;
    }

    void applyTrafficSimulation(TrafficMode mode) {
        double multiplier = 1.0;
        if (mode == TRAFFIC_PEAK_HOUR) multiplier = 1.4;
        else if (mode == TRAFFIC_HEAVY) multiplier = 2.2;

        for (int i = 0; i < num_vertices; i++) {
            AdjNode* curr = array[i].head;
            while (curr) {
                curr->current_weight = static_cast<int>(curr->base_weight * multiplier);
                curr = curr->next;
            }
        }
    }

    int getHeuristic(int u, int v) const {
        int dx = locations[u].x - locations[v].x;
        int dy = locations[u].y - locations[v].y;
        return static_cast<int>(sqrt(dx * dx + dy * dy));
    }
};

// 4. PATHFINDING CORE ALGORITHMS
vector<int> buildPathSequence(int target, const vector<int>& parent) {
    vector<int> path;
    for (int v = target; v != -1; v = parent[v]) {
        path.push_back(v);
    }
    reverse(path.begin(), path.end());
    return path;
}

void runBFS(const Graph& graph, int source, int target, vector<int>& path, PerformanceMetrics& metrics) {
    clock_t start_time = clock();
    int V = graph.num_vertices;
    vector<int> parent(V, -1);
    vector<bool> visited(V, false);

    queue<int> q;
    visited[source] = true;
    q.push(source);

    metrics.nodes_explored = 0;
    metrics.routes_skipped = 0;
    bool found = false;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        metrics.nodes_explored++;

        if (u == target) { found = true; break; }

        AdjNode* adj = graph.array[u].head;
        while (adj) {
            int v = adj->vertex;
            if (globalExclusionTree.rbt_search(u, v)) {
                metrics.routes_skipped++;
                adj = adj->next;
                continue;
            }
            if (!visited[v]) {
                visited[v] = true;
                parent[v] = u;
                q.push(v);
            }
            adj = adj->next;
        }
    }

    if (found) {
        path = buildPathSequence(target, parent);
        metrics.hop_count = static_cast<int>(path.size() - 1);
        
        int total_cost = 0;
        for (size_t i = 0; i < path.size() - 1; i++) {
            int curr_u = path[i];
            int next_v = path[i+1];
            AdjNode* edge = graph.array[curr_u].head;
            int edge_w = INF;
            while (edge) {
                if (edge->vertex == next_v) {
                    edge_w = edge->current_weight;
                    break;
                }
                edge = edge->next;
            }
            if (edge_w == INF) { total_cost = INF; break; }
            total_cost += edge_w;
        }
        metrics.path_cost = total_cost;
    } else {
        path.clear();
        metrics.path_cost = INF;
        metrics.hop_count = 0;
    }
    metrics.execution_time_ms = (static_cast<double>(clock() - start_time) / CLOCKS_PER_SEC) * 1000.0;
    metrics.memory_used_bytes = (V * sizeof(int)) + (V * sizeof(bool));
}

void runDijkstra(const Graph& graph, int source, int target, vector<int>& path, PerformanceMetrics& metrics) {
    clock_t start_time = clock();
    int V = graph.num_vertices;
    vector<int> dist(V, INF);
    vector<int> parent(V, -1);
    vector<bool> closed(V, false);

    MinHeap min_heap(V);
    dist[source] = 0;
    min_heap.insert(source, 0, 0);

    metrics.nodes_explored = 0;
    metrics.routes_skipped = 0;

    while (!min_heap.empty()) {
        HeapNode curr = min_heap.extractMin();
        int u = curr.vertex;
        
        if (closed[u]) continue;
        closed[u] = true;
        metrics.nodes_explored++;

        if (u == target) break;

        AdjNode* adj = graph.array[u].head;
        while (adj) {
            int v = adj->vertex;
            if (globalExclusionTree.rbt_search(u, v)) {
                metrics.routes_skipped++;
                adj = adj->next;
                continue;
            }
            if (!closed[v] && dist[u] != INF && dist[u] + adj->current_weight < dist[v]) {
                dist[v] = dist[u] + adj->current_weight;
                parent[v] = u;
                min_heap.decreaseKey(v, dist[v], dist[v]);
            }
            adj = adj->next;
        }
    }

    if (dist[target] != INF) {
        path = buildPathSequence(target, parent);
        metrics.path_cost = dist[target];
        metrics.hop_count = static_cast<int>(path.size() - 1);
    } else {
        path.clear();
        metrics.path_cost = INF;
        metrics.hop_count = 0;
    }
    metrics.execution_time_ms = (static_cast<double>(clock() - start_time) / CLOCKS_PER_SEC) * 1000.0;
    metrics.memory_used_bytes = (V * sizeof(int) * 2) + (V * sizeof(bool)) + (V * sizeof(HeapNode));
}

void runAStar(const Graph& graph, int source, int target, vector<int>& path, PerformanceMetrics& metrics) {
    clock_t start_time = clock();
    int V = graph.num_vertices;
    vector<int> dist(V, INF);
    vector<int> parent(V, -1);
    vector<bool> closed(V, false);

    MinHeap min_heap(V);
    dist[source] = 0;
    min_heap.insert(source, graph.getHeuristic(source, target), 0);

    metrics.nodes_explored = 0;
    metrics.routes_skipped = 0;

    while (!min_heap.empty()) {
        HeapNode curr = min_heap.extractMin();
        int u = curr.vertex;

        if (closed[u]) continue;
        closed[u] = true;
        metrics.nodes_explored++;

        if (u == target) break;

        AdjNode* adj = graph.array[u].head;
        while (adj) {
            int v = adj->vertex;
            if (globalExclusionTree.rbt_search(u, v)) {
                metrics.routes_skipped++;
                adj = adj->next;
                continue;
            }
            int tentative_g = dist[u] + adj->current_weight;
            if (!closed[v] && tentative_g < dist[v]) {
                dist[v] = tentative_g;
                parent[v] = u;
                int f_cost = tentative_g + graph.getHeuristic(v, target);
                min_heap.decreaseKey(v, f_cost, tentative_g);
            }
            adj = adj->next;
        }
    }

    if (dist[target] != INF) {
        path = buildPathSequence(target, parent);
        metrics.path_cost = dist[target];
        metrics.hop_count = static_cast<int>(path.size() - 1);
    } else {
        path.clear();
        metrics.path_cost = INF;
        metrics.hop_count = 0;
    }
    metrics.execution_time_ms = (static_cast<double>(clock() - start_time) / CLOCKS_PER_SEC) * 1000.0;
    metrics.memory_used_bytes = (V * sizeof(int) * 2) + (V * sizeof(bool)) + (V * sizeof(HeapNode));
}

// 5. INPUT INGESTION & DATA UTILITIES
Graph* generateGridGraph(int width, int height) {
    int vertices = width * height;
    Graph* g = new Graph(vertices, "Mesh Grid");
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            int u = r * width + c;
            g->locations[u].x = c * 50;
            g->locations[u].y = r * 50;
            if (c + 1 < width) {
                int v = u + 1;
                int w = rand() % 20 + 5;
                g->addEdge(u, v, w);
                g->addEdge(v, u, w); // Bidirectional implementation
            }
            if (r + 1 < height) {
                int v = u + width;
                int w = rand() % 20 + 5;
                g->addEdge(u, v, w);
                g->addEdge(v, u, w); // Bidirectional implementation
            }
        }
    }
    return g;
}

Graph* loadGraphFromCSV(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cout << "  [Error] Could not open CSV file: " << filepath << "\n";
        return nullptr;
    }

    string line;
    int max_node_id = -1;
    struct RowEdge { int src, dst, cost; };
    vector<RowEdge> temporary_edges;
    int line_count = 0;

    while (getline(file, line)) {
        line_count++;
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        stringstream ss(line);
        string s_u, s_v, s_w;

        if (getline(ss, s_u, ',') && getline(ss, s_v, ',') && getline(ss, s_w, ',')) {
            try {
                int u = stoi(s_u);
                int v = stoi(s_v);
                int w = stoi(s_w);

                if (u < 0 || v < 0 || w < 0) continue;
                if (u > max_node_id) max_node_id = u;
                if (v > max_node_id) max_node_id = v;
                temporary_edges.push_back({u, v, w});
            } catch (...) {
                continue;
            }
        }
    }
    file.close();

    if (temporary_edges.empty()) {
        cout << "  [Error] No valid edges found in CSV file.\n";
        return nullptr;
    }

    Graph* g = new Graph(max_node_id + 1, "CSV File");
    for (const auto& edge : temporary_edges) {
        g->addEdge(edge.src, edge.dst, edge.cost);
    }
    cout << "  [Success] Loaded " << g->num_vertices << " vertices from CSV file.\n";
    return g;
}

// 6. ENHANCED STOCHASTIC BENCHMARKING FRAMEWORK
void runBenchmarkSuite(const Graph& graph) {
    const int NUM_PAIRS = 10;
    cout << "\n ALGORITHM BENCHMARK RESULTS \n";
    cout << "  Graph Type : " << graph.type_label << " | Node Count: " << graph.num_vertices << "\n";
    cout << "  Sampling Profile: " << NUM_PAIRS << " uniform random coordinate evaluation pairs\n";
    cout << "\n";

    double avg_bfs_cost = 0, avg_bfs_nodes = 0, avg_bfs_skipped = 0, avg_bfs_mem = 0, avg_bfs_time = 0;
    double avg_dijk_cost = 0, avg_dijk_nodes = 0, avg_dijk_skipped = 0, avg_dijk_mem = 0, avg_dijk_time = 0;
    double avg_astar_cost = 0, avg_astar_nodes = 0, avg_astar_skipped = 0, avg_astar_mem = 0, avg_astar_time = 0;

    int completed_runs = 0;

    for (int i = 0; i < NUM_PAIRS; i++) {
        int src = rand() % graph.num_vertices;
        int dst = rand() % graph.num_vertices;
        if (src == dst && graph.num_vertices > 1) {
            dst = (src + 1) % graph.num_vertices;
        }

        vector<int> path;
        PerformanceMetrics m_bfs, m_dijk, m_astar;

        runBFS(graph, src, dst, path, m_bfs);
        runDijkstra(graph, src, dst, path, m_dijk);
        runAStar(graph, src, dst, path, m_astar);

        avg_bfs_cost += (m_bfs.path_cost == INF ? 0 : m_bfs.path_cost);
        avg_bfs_nodes += m_bfs.nodes_explored;
        avg_bfs_skipped += m_bfs.routes_skipped;
        avg_bfs_mem += m_bfs.memory_used_bytes;
        avg_bfs_time += m_bfs.execution_time_ms;

        avg_dijk_cost += (m_dijk.path_cost == INF ? 0 : m_dijk.path_cost);
        avg_dijk_nodes += m_dijk.nodes_explored;
        avg_dijk_skipped += m_dijk.routes_skipped;
        avg_dijk_mem += m_dijk.memory_used_bytes;
        avg_dijk_time += m_dijk.execution_time_ms;

        avg_astar_cost += (m_astar.path_cost == INF ? 0 : m_astar.path_cost);
        avg_astar_nodes += m_astar.nodes_explored;
        avg_astar_skipped += m_astar.routes_skipped;
        avg_astar_mem += m_astar.memory_used_bytes;
        avg_astar_time += m_astar.execution_time_ms;

        completed_runs++;
    }

    if (completed_runs > 0) {
        avg_bfs_cost /= completed_runs; avg_bfs_nodes /= completed_runs; avg_bfs_skipped /= completed_runs; avg_bfs_mem /= completed_runs; avg_bfs_time /= completed_runs;
        avg_dijk_cost /= completed_runs; avg_dijk_nodes /= completed_runs; avg_dijk_skipped /= completed_runs; avg_dijk_mem /= completed_runs; avg_dijk_time /= completed_runs;
        avg_astar_cost /= completed_runs; avg_astar_nodes /= completed_runs; avg_astar_skipped /= completed_runs; avg_astar_mem /= completed_runs; avg_astar_time /= completed_runs;
    }

    cout << left << setw(18) << "Algorithm (Avg)" << " | "
         << setw(12) << "Path Cost" << " | "
         << setw(16) << "Nodes Explored" << " | "
         << setw(16) << "Routes Skipped" << " | "
         << setw(16) << "Memory (Bytes)" << " | "
         << setw(12) << "Time (ms)" << "\n";
    cout << "\n";

    auto printAvgRow = [](const string& name, double cost, double nodes, double skipped, double mem, double time) {
        cout << left << setw(18) << name << " | "
             << setw(12) << fixed << setprecision(1) << cost << " | "
             << setw(16) << setprecision(1) << nodes << " | "
             << setw(16) << setprecision(1) << skipped << " | "
             << setw(16) << setprecision(1) << mem << " | "
             << setw(12) << setprecision(4) << time << "\n";
    };

    printAvgRow("BFS (Unweighted)", avg_bfs_cost, avg_bfs_nodes, avg_bfs_skipped, avg_bfs_mem, avg_bfs_time);
    printAvgRow("Dijkstra", avg_dijk_cost, avg_dijk_nodes, avg_dijk_skipped, avg_dijk_mem, avg_dijk_time);
    printAvgRow("A* Search", avg_astar_cost, avg_astar_nodes, avg_astar_skipped, avg_astar_mem, avg_astar_time);
    cout << "\n";

    double speedup = avg_dijk_time / (avg_astar_time + 1e-9);
    double node_reduction = 0.0;
    if (avg_dijk_nodes > 0) {
        node_reduction = ((avg_dijk_nodes - avg_astar_nodes) / avg_dijk_nodes) * 100.0;
    }

    cout << "\n PERFORMANCE ANALYTICS \n";
    cout << "  • A* reduction workspace: " << fixed << setprecision(2) << (node_reduction < 0 ? 0.0 : node_reduction) << "% fewer node settlements than Dijkstra.\n";
    cout << "  • A* acceleration ratio : " << speedup << "x faster traversal efficiency relative to Dijkstra.\n";
    cout << "\n";
}

// 7. DASHBOARD TELEMETRY MONITOR
void displayDashboard(const string& alg, int cost, int visited, double time_ms, TrafficMode traffic, int hops, int skipped) {
    cout << "\n\n";
    cout << "                     ROUTE METRICS DASHBOARD            \n";
    cout << "\n";
    cout << "  • Algorithm Name       : " << alg << "\n";
    cout << "  • Traffic Mode         : " << (traffic == TRAFFIC_NORMAL ? "NORMAL" : traffic == TRAFFIC_PEAK_HOUR ? "PEAK HOUR" : "HEAVY TRAFFIC") << "\n";
    cout << "  • Excluded Edges Count : " << globalExclusionTree.getCount() << "\n";
    cout << "  • Bypassed Edges Count : " << skipped << "\n";
    cout << "  • Path Cost            : " << (cost == INF ? -1 : cost) << "\n";
    cout << "  • Total Hop Count      : " << hops << "\n";
    cout << "  • Explored Node Count  : " << visited << "\n";
    cout << "  • Execution Latency    : " << fixed << setprecision(4) << time_ms << " ms\n";
    
}

// 8. SYSTEM EXECUTIVE CONTROLLER
int main() {
    srand(1337); 
    int current_dim = 20;
    Graph* current_graph = generateGridGraph(current_dim, current_dim);
    
    int menu_input;
    TrafficMode current_traffic = TRAFFIC_NORMAL;

    int last_known_src = -1;
    int last_known_dst = -1;
    vector<int> last_path;
    int last_cost = INF;

    auto triggerDynamicRecalculation = [&]() {
        if (last_known_src == -1 || last_known_dst == -1) return;
        
        cout << "\n             DYNAMIC ROUTE RECALCULATION              \n";
        cout << "  [System Notice] Topology structural variance detected. Syncing updates...\n";
        
        vector<int> new_path;
        PerformanceMetrics new_metrics;
        runAStar(*current_graph, last_known_src, last_known_dst, new_path, new_metrics);

        cout << "  Previous Route Cost: " << (last_cost == INF ? -1 : last_cost) << "\n";
        cout << "  Updated Route Cost : " << (new_metrics.path_cost == INF ? -1 : new_metrics.path_cost) << "\n";
        cout << "  Cost Difference    : ";
        if (last_cost == INF || new_metrics.path_cost == INF) cout << "N/A\n";
        else cout << (new_metrics.path_cost - last_cost) << "\n";
        
        cout << "  Previous Path: ";
        if (last_path.empty()) cout << "None\n";
        else {
            for (size_t i = 0; i < last_path.size(); i++) {
                cout << last_path[i] << (i + 1 < last_path.size() ? " -> " : "");
            }
            cout << "\n";
        }

        cout << "  Updated Path : ";
        if (new_path.empty()) cout << "None\n";
        else {
            for (size_t i = 0; i < new_path.size(); i++) {
                cout << new_path[i] << (i + 1 < new_path.size() ? " -> " : "");
            }
            cout << "\n";
        }

        last_path = new_path;
        last_cost = new_metrics.path_cost;
    };

    while (true) {
        cout << "\n    OPTIMAL ROUTE MANAGEMENT SYSTEM \n";
        cout << "1. Load Graph from CSV\n";
        cout << "2. Generate Grid Graph\n";
        cout << "3. Add Avoided Route\n";
        cout << "4. Remove Avoided Route\n";
        cout << "5. View Avoided Routes\n";
        cout << "6. Change Traffic Mode\n";
        cout << "7. Find Route\n";
        cout << "8. Run Benchmark\n";
        cout << "9. Exit\n";
        cout << "Select Operation Mode: ";
        if (!(cin >> menu_input)) break;

        if (menu_input == 1) {
            string path_target;
            cout << "Enter filepath to CSV file source: ";
            cin >> path_target;
            Graph* incoming = loadGraphFromCSV(path_target);
            if (incoming != nullptr) {
                delete current_graph;
                current_graph = incoming;
                last_known_src = -1; 
                last_known_dst = -1;
                last_path.clear();
                last_cost = INF;
            }
        } else if (menu_input == 2) {
            cout << "Enter square dimension layout bound size: ";
            cin >> current_dim;
            if (current_dim > 1) {
                delete current_graph;
                current_graph = generateGridGraph(current_dim, current_dim);
                last_known_src = -1;
                last_known_dst = -1;
                last_path.clear();
                last_cost = INF;
                cout << "  Bidirectional mesh grid graph generated successfully.\n";
            }
        } else if (menu_input == 3) {
            int u, v;
            cout << "Enter source and destination node IDs to avoid (u v): ";
            cin >> u >> v;
            globalExclusionTree.rbt_insert(u, v);
            cout << "  Route edge added to Red-Black Tree avoidance repository.\n";
            triggerDynamicRecalculation();
        } else if (menu_input == 4) {
            int u, v;
            cout << "Enter source and destination node IDs to restore (u v): ";
            cin >> u >> v;
            globalExclusionTree.rbt_delete(u, v);
            cout << "  Route edge removed from Red-Black Tree exclusion rules.\n";
            triggerDynamicRecalculation();
        } else if (menu_input == 5) {
            cout << "\n--- REGISTERED TREE EXCLUSIONS ACTIVE ---\n";
            globalExclusionTree.listExclusions();
        } else if (menu_input == 6) {
            int option;
            cout << "Choose Traffic Level (0: Normal, 1: Peak Hour, 2: Heavy Traffic): ";
            cin >> option;
            if (option >= 0 && option <= 2) {
                current_traffic = static_cast<TrafficMode>(option);
                current_graph->applyTrafficSimulation(current_traffic);
                cout << "  Network matrix weights scaled to simulate chosen traffic profile.\n";
                triggerDynamicRecalculation();
            } else {
                cout << "  [Error] Selection exceeds available profile bounds.\n";
            }
        } else if (menu_input == 7) {
            int src, dst;
            cout << "Enter Origin Node ID and Target Node ID (0 to " << current_graph->num_vertices - 1 << "): ";
            cin >> src >> dst;

            if (src < 0 || src >= current_graph->num_vertices || dst < 0 || dst >= current_graph->num_vertices) {
                cout << "  [Error] User parameters fall outside of loaded array limits.\n";
                continue;
            }

            last_known_src = src;
            last_known_dst = dst;

            PerformanceMetrics tracking;
            runAStar(*current_graph, src, dst, last_path, tracking);
            last_cost = tracking.path_cost;

            displayDashboard("A* Search", tracking.path_cost, static_cast<int>(tracking.nodes_explored), tracking.execution_time_ms, current_traffic, tracking.hop_count, static_cast<int>(tracking.routes_skipped));

            if (!last_path.empty()) {
                cout << "  Confirmed Path Output Sequence: ";
                for (size_t i = 0; i < last_path.size(); i++) {
                    cout << last_path[i] << (i + 1 < last_path.size() ? " -> " : "");
                }
                cout << "\n";
            } else {
                cout << "  No path vector discovered connecting specified target boundaries.\n";
            }
        } else if (menu_input == 8) {
            runBenchmarkSuite(*current_graph);
        } else {
            break;
        }
    }

    delete current_graph;
    return 0;
}