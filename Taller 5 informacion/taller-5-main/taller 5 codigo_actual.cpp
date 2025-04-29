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

#define N 5
#define INF 9999

mutex mtx;
atomic<int> active_threads(0);

// Generar matriz de costos aleatorios
vector<vector<int>> generateCostMatrix(N, int min_val = 1, int max_val = 10) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(min_val, max_val);
    
    vector<vector<int>> matrix(n, vector<int>(n));
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            matrix[i][j] = (i == j) ? 0 : dis(gen);
        }
    }
    return matrix;
}

struct BestPath {
    int cost = numeric_limits<int>::max();
    vector<int> path;
};

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
        if(matrix[current][i] != 0 && !visited[i]) {
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
        
        // Mostrar matriz de costos
        cout << "Matriz de costos:\n";
        for(const auto& row : matrix) {
            for(int val : row) cout << val << "\t";
            cout << endl;
        }
        
        // Generar nodos inicio y fin aleatorios
        uniform_int_distribution<> dis_node(0, n-1);
        int start = dis_node(gen);
        int end = dis_node(gen);
        while(end == start) end = dis_node(gen);
        
        cout << "Nodo inicio: " << start << ", Nodo fin: " << end << endl;
        
        atomic<int> minDist(numeric_limits<int>::max());
        BestPath best;
        vector<bool> visited(n, false);
        visited[start] = true;
        active_threads = 0;
        
        auto start_time = high_resolution_clock::now();
        
        vector<int> initial_path;
        parallelBacktracking(matrix, start, end, 0, minDist, best, visited, initial_path);
        
        // Esperar a que todos los hilos terminen
        while(active_threads > 0) {
            this_thread::sleep_for(milliseconds(10));
        }
        
        auto end_time = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end_time - start_time);
        
        cout << "Mejor camino: ";
        for(int node : best.path) cout << node << " ";
        cout << "| Costo: " << best.cost << " | Tiempo: " << duration.count() << " ms\n";
    }
    
    return 0;
}