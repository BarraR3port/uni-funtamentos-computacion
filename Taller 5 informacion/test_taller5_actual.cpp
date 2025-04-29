#include <iostream>
#include <vector>
#include <random>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <omp.h>
#include <iomanip>
#include <functional>

using namespace std;
using namespace std::chrono;

#define INF 9999

struct BestPath {
    atomic<int> cost;
    vector<int> path;
    mutex mtx;

    BestPath() : cost(numeric_limits<int>::max()) {}

    void update(int new_cost, const vector<int>& new_path) {
        int current = cost.load();
        while(new_cost < current) {
            if(cost.compare_exchange_weak(current, new_cost)) {
                lock_guard<mutex> lock(mtx);
                path = new_path;
                break;
            }
        }
    }
};

vector<vector<int>> generateCostMatrix(int n, int min_val = 1, int max_val = 100, double inf_prob = 0.3) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis_val(min_val, max_val);
    uniform_real_distribution<> dis_prob(0.0, 1.0);

    vector<vector<int>> matrix(n, vector<int>(n, 0));
    
    for(int i = 0; i < n; i++) {
        for(int j = i+1; j < n; j++) {
            if(dis_prob(gen) < inf_prob) {
                matrix[i][j] = INF;
                matrix[j][i] = INF;
            } else {
                int val = dis_val(gen);
                matrix[i][j] = val;
                matrix[j][i] = val;
            }
        }
    }
    
    for(int i = 0; i < n; i++) {
        matrix[i][i] = 0;
    }
    
    return matrix;
}

int findMinEdge(const vector<vector<int>>& matrix) {
    int min_val = INF;
    for(const auto& row : matrix) {
        for(int val : row) {
            if(val != 0 && val < min_val) {
                min_val = val;
            }
        }
    }
    return min_val;
}

void sequentialBacktracking(const vector<vector<int>>& matrix, int current, int end,
                          int dist, BestPath& best, vector<bool>& visited, vector<int>& path, int min_edge) {
    path.push_back(current);
    
    // Poda más agresiva considerando el mínimo posible
    if(dist + min_edge >= best.cost) {
        path.pop_back();
        return;
    }

    if(current == end) {
        best.update(dist, path);
        path.pop_back();
        return;
    }

    for(int i = 0; i < matrix.size(); i++) {
        if(matrix[current][i] != 0 && matrix[current][i] != INF && !visited[i]) {
            visited[i] = true;
            sequentialBacktracking(matrix, i, end, dist + matrix[current][i], best, visited, path, min_edge);
            visited[i] = false;
        }
    }
    path.pop_back();
}

void parallelBacktrackingWrapper(const vector<vector<int>>& matrix, int start, int end, BestPath& best) {
    int n = matrix.size();
    int min_edge = findMinEdge(matrix);
    
    // Solo paralelizar para matrices grandes
    if(n <= 15) {
        vector<bool> visited(n, false);
        visited[start] = true;
        vector<int> path = {start};
        sequentialBacktracking(matrix, start, end, 0, best, visited, path, min_edge);
        return;
    }

    vector<bool> visited(n, false);
    visited[start] = true;
    vector<int> base_path = {start};

    #pragma omp parallel for schedule(dynamic)
    for(int i = 0; i < n; i++) {
        if(matrix[start][i] != 0 && matrix[start][i] != INF && !visited[i]) {
            vector<bool> local_visited = visited;
            local_visited[i] = true;
            vector<int> path = base_path;
            path.push_back(i);

            function<void(int, int, vector<bool>&, vector<int>&)> backtrack;
            backtrack = [&](int node, int current_dist, vector<bool>& local_vis, vector<int>& local_path) {
                if(current_dist + min_edge >= best.cost) return;

                if(node == end) {
                    best.update(current_dist, local_path);
                    return;
                }

                for(int j = 0; j < n; j++) {
                    if(matrix[node][j] != 0 && matrix[node][j] != INF && !local_vis[j]) {
                        if(current_dist + matrix[node][j] + min_edge >= best.cost) continue;
                        
                        local_vis[j] = true;
                        local_path.push_back(j);
                        backtrack(j, current_dist + matrix[node][j], local_vis, local_path);
                        local_path.pop_back();
                        local_vis[j] = false;
                    }
                }
            };

            backtrack(i, matrix[start][i], local_visited, path);
        }
    }
}

int main() {
    const int num_threads = omp_get_max_threads();
    omp_set_num_threads(num_threads);
    cout << "Configuración OpenMP - Hilos: " << num_threads << "\n\n";

    const int min_size = 2;
    const int max_size = 50;
    random_device rd;
    mt19937 gen(rd());

    for(int n = min_size; n <= max_size; n++) {
        cout << "=== Matriz " << n << "x" << n << " ===\n";
        auto matrix = generateCostMatrix(n);
        
        if(n <= 10) {
            cout << "Matriz de costos:\n";
            for(const auto& row : matrix) {
                for(int val : row) {
                    if(val == INF) cout << "INF\t";
                    else cout << val << "\t";
                }
                cout << endl;
            }
        }

        uniform_int_distribution<> dis_node(0, n - 1);
        int start = dis_node(gen);
        int end = dis_node(gen);
        while(end == start) end = dis_node(gen);

        cout << "Nodo inicio: " << start << ", Nodo fin: " << end << "\n";

        // Ejecución secuencial
        BestPath seq_best;
        vector<bool> seq_visited(n, false);
        seq_visited[start] = true;
        vector<int> seq_path;
        int min_edge = findMinEdge(matrix);

        auto seq_start = high_resolution_clock::now();
        sequentialBacktracking(matrix, start, end, 0, seq_best, seq_visited, seq_path, min_edge);
        auto seq_end = high_resolution_clock::now();
        auto seq_duration = duration_cast<microseconds>(seq_end - seq_start);
        cout << "Secuencial - Costo: " << seq_best.cost << " | Tiempo: " << seq_duration.count() << " μs\n";

        // Ejecución paralela
        BestPath par_best;
        auto par_start = high_resolution_clock::now();
        parallelBacktrackingWrapper(matrix, start, end, par_best);
        auto par_end = high_resolution_clock::now();
        auto par_duration = duration_cast<microseconds>(par_end - par_start);
        cout << "Paralelo   - Costo: " << par_best.cost << " | Tiempo: " << par_duration.count() << " μs\n";

        if(seq_duration.count() > 0 && par_duration.count() > 0) {
            double speedup = static_cast<double>(seq_duration.count()) / par_duration.count();
            cout << fixed << setprecision(2);
            cout << "Speedup: " << speedup << "x\n";
        }

        if(seq_best.cost != par_best.cost) {
            cerr << "¡Advertencia: Los resultados difieren!\n";
            cerr << "Secuencial: ";
            for(int n : seq_best.path) cerr << n << " ";
            cerr << "\nParalelo:   ";
            for(int n : par_best.path) cerr << n << " ";
            cerr << "\n";
        }

        cout << "\n";
    }

    return 0;
}