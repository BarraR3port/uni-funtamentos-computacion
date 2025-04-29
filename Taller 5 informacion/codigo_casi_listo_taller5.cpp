#include <iostream>
#include <vector>
#include <random>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <algorithm>

using namespace std;
using namespace std::chrono;

#define INF 9999

mutex mtx;
atomic<int> active_threads(0);

// Generar matriz de costos aleatorios simétrica
vector<vector<int>> generateCostMatrix(int n, int min_val = 1, int max_val = 10, double inf_prob = 0.05) {
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
    
    // Asegurar diagonal de ceros
    for(int i = 0; i < n; i++) {
        matrix[i][i] = 0;
    }
    
    return matrix;
}

struct BestPath {
    int cost = numeric_limits<int>::max();
    vector<int> path;
};

// Versión secuencial del backtracking
void sequentialBacktracking(const vector<vector<int>>& matrix, int current, int end, 
                          int dist, BestPath& best, vector<bool>& visited, vector<int>& path) {
    path.push_back(current);
    
    if(current == end) {
        if(dist < best.cost) {
            best.cost = dist;
            best.path = path;
        }
        path.pop_back();
        return;
    }
    
    for(int i = 0; i < matrix.size(); i++) {
        if(matrix[current][i] != 0 && matrix[current][i] != INF && !visited[i]) {
            visited[i] = true;
            sequentialBacktracking(matrix, i, end, dist + matrix[current][i], best, visited, path);
            visited[i] = false;
        }
    }
    
    path.pop_back();
}

// Versión paralela del backtracking
void parallelBacktracking(const vector<vector<int>>& matrix, int current, int end, 
                        int dist, atomic<int>& minDist, BestPath& best, 
                        vector<bool> visited, vector<int> path, int depth = 0) {
    path.push_back(current);
    
    if(current == end) {
        lock_guard<mutex> lock(mtx);
        if(dist < best.cost) {
            best.cost = dist;
            best.path = path;
            minDist = dist;
        }
        return;
    }
    
    vector<thread> threads;
    for(int i = 0; i < matrix.size(); i++) {
        if(matrix[current][i] != 0 && matrix[current][i] != INF && !visited[i]) {
            visited[i] = true;
            
            if(depth < 2) { // Límite de profundidad para paralelización
                active_threads++;
                threads.emplace_back([&matrix, i, end, dist, &minDist, &best, visited, path, current, depth]() {
                    int localDist = dist + matrix[current][i];
                    parallelBacktracking(matrix, i, end, localDist, minDist, best, visited, path, depth + 1);
                    active_threads--;
                });
            } else {
                parallelBacktracking(matrix, i, end, dist + matrix[current][i], minDist, best, visited, path, depth + 1);
            }
            
            visited[i] = false;
        }
    }
    
    for(auto& t : threads) {
        if(t.joinable()) t.join();
    }
}

int main() {
    random_device rd;
    mt19937 gen(rd());
    
    // Ciclo desde matrices 2x2 hasta 20x20
    for(int n = 2; n <= 20; n++) {
        cout << "\nMatriz de " << n << "x" << n << ":\n";
        
        auto matrix = generateCostMatrix(n);
        
        // Mostrar matriz de costos (siempre)
        cout << "Matriz de costos:\n";
        for(const auto& row : matrix) {
            for(int val : row) {
                if(val == INF) cout << "INF\t";
                else cout << val << "\t";
            }
            cout << endl;
        }
        
        // Generar nodos inicio y fin aleatorios
        uniform_int_distribution<> dis_node(0, n-1);
        int start = dis_node(gen);
        int end = dis_node(gen);
        while(end == start) end = dis_node(gen);
        
        cout << "Nodo inicio: " << start << ", Nodo fin: " << end << endl;
        
        // Ejecución secuencial
        BestPath seq_best;
        vector<bool> seq_visited(n, false);
        seq_visited[start] = true;
        vector<int> seq_path;
        
        auto seq_start = high_resolution_clock::now();
        sequentialBacktracking(matrix, start, end, 0, seq_best, seq_visited, seq_path);
        auto seq_end = high_resolution_clock::now();
        auto seq_duration = duration_cast<microseconds>(seq_end - seq_start);
        
        cout << "Secuencial - Mejor camino: ";
        for(int node : seq_best.path) cout << node << " ";
        cout << "| Costo: " << seq_best.cost << " | Tiempo: " << seq_duration.count() << " μs\n";
        
        // Ejecución paralela
        atomic<int> par_minDist(numeric_limits<int>::max());
        BestPath par_best;
        vector<bool> par_visited(n, false);
        par_visited[start] = true;
        active_threads = 0;
        vector<int> par_path;
        
        auto par_start = high_resolution_clock::now();
        parallelBacktracking(matrix, start, end, 0, par_minDist, par_best, par_visited, par_path);
        
        // Esperar a que todos los hilos terminen
        while(active_threads > 0) {
            this_thread::sleep_for(microseconds(10));
        }
        
        auto par_end = high_resolution_clock::now();
        auto par_duration = duration_cast<microseconds>(par_end - par_start);
        
        cout << "Paralelo   - Mejor camino: ";
        for(int node : par_best.path) cout << node << " ";
        cout << "| Costo: " << par_best.cost << " | Tiempo: " << par_duration.count() << " μs\n";
        
        // Calcular speedup
        if(seq_duration.count() > 0 && par_duration.count() > 0) {
            double speedup = static_cast<double>(seq_duration.count()) / par_duration.count();
            cout << "Speedup: " << speedup << "x\n";
        }
    }
    
    return 0;
}