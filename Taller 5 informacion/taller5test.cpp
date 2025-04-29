#include <iostream>
#include <vector>
#include <random>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <omp.h>
#include <iomanip>

using namespace std;
using namespace std::chrono;

#define INF 9999

// Estructura para almacenar el mejor camino encontrado
struct BestPath {
    int cost = numeric_limits<int>::max();
    vector<int> path;
    mutex mtx;

    void update(int new_cost, const vector<int>& new_path) {
        lock_guard<mutex> lock(mtx);
        if(new_cost < cost) {
            cost = new_cost;
            path = new_path;
        }
    }
};

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
    
    for(int i = 0; i < n; i++) {
        matrix[i][i] = 0;
    }
    
    return matrix;
}

// Versión secuencial del backtracking
void sequentialBacktracking(const vector<vector<int>>& matrix, int current, int end, 
                          int dist, BestPath& best, vector<bool>& visited, vector<int>& path) {
    path.push_back(current);
    
    // Poda: si ya superamos el mejor costo, no continuar
    if(dist >= best.cost) {
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
            sequentialBacktracking(matrix, i, end, dist + matrix[current][i], best, visited, path);
            visited[i] = false;
        }
    }
    
    path.pop_back();
}

// Versión paralela optimizada del backtracking con OpenMP
void parallelBacktracking(const vector<vector<int>>& matrix, int current, int end, 
                        int dist, BestPath& best, vector<bool> visited, vector<int> path, int depth = 0) {
    path.push_back(current);
    
    // Poda más agresiva
    if(dist >= best.cost) {
        return;
    }
    
    if(current == end) {
        best.update(dist, path);
        return;
    }
    
    // Estrategia híbrida: solo paralelizar para casos que lo justifiquen
    if(matrix.size() > 8 && depth < 3) {  // Ajustar estos valores según necesidades
        #pragma omp parallel for schedule(dynamic)
        for(int i = 0; i < matrix.size(); i++) {
            if(matrix[current][i] != 0 && matrix[current][i] != INF && !visited[i]) {
                vector<bool> new_visited = visited;
                vector<int> new_path = path;
                new_visited[i] = true;
                int new_dist = dist + matrix[current][i];
                
                parallelBacktracking(matrix, i, end, new_dist, best, new_visited, new_path, depth + 1);
            }
        }
    } else {
        // Ejecución secuencial para casos pequeños o mucha profundidad
        for(int i = 0; i < matrix.size(); i++) {
            if(matrix[current][i] != 0 && matrix[current][i] != INF && !visited[i]) {
                visited[i] = true;
                parallelBacktracking(matrix, i, end, dist + matrix[current][i], best, visited, path, depth + 1);
                visited[i] = false;
            }
        }
    }
}

int main() {
    random_device rd;
    mt19937 gen(rd());
    
    // Configuración de OpenMP
    const int num_threads = 4;  // Ajustar según el número de cores disponibles
    omp_set_num_threads(num_threads);
    cout << "Configuración OpenMP - Hilos: " << num_threads << "\n\n";
    
    // Rango de tamaños de matriz a evaluar
    const int min_size = 2;
    const int max_size = 20;
    
    for(int n = min_size; n <= max_size; n++) {
        cout << "=== Matriz " << n << "x" << n << " ===\n";
        
        // Generar matriz de costos
        auto matrix = generateCostMatrix(n);
        
        // Mostrar matriz (opcional para n pequeños)
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
        
        cout << "Secuencial - Costo: " << seq_best.cost << " | Tiempo: " << seq_duration.count() << " μs\n";
        
        // Ejecución paralela
        BestPath par_best;
        vector<bool> par_visited(n, false);
        par_visited[start] = true;
        vector<int> par_path;
        
        auto par_start = high_resolution_clock::now();
        parallelBacktracking(matrix, start, end, 0, par_best, par_visited, par_path);
        auto par_end = high_resolution_clock::now();
        auto par_duration = duration_cast<microseconds>(par_end - par_start);
        
        cout << "Paralelo   - Costo: " << par_best.cost << " | Tiempo: " << par_duration.count() << " μs\n";
        
        // Calcular speedup
        if(seq_duration.count() > 0 && par_duration.count() > 0) {
            double speedup = static_cast<double>(seq_duration.count()) / par_duration.count();
            cout << fixed << setprecision(2);
            cout << "Speedup: " << speedup << "x\n";
        }
        
        // Verificar consistencia de resultados
        if(seq_best.cost != par_best.cost) {
            cerr << "¡Advertencia: Los resultados secuencial y paralelo difieren!\n";
        }
        
        cout << endl;
    }
    
    return 0;
}


ERROR!
/tmp/Vy2hKGxdRp/main.cpp: In function 'void parallelBacktrackingWrapper(const std::vector<std::vector<int> >&, int, int, BestPath&)':
/tmp/Vy2hKGxdRp/main.cpp:96:13: error: 'function' was not declared in this scope
   96 |             function<void(int, int, vector<bool>&, vector<int>&)> backtrack;
      |             ^~~~~~~~
ERROR!
/tmp/Vy2hKGxdRp/main.cpp:10:1: note: 'std::function' is defined in header '<functional>'; this is probably fixable by adding '#include <functional>'
    9 | #include <iomanip>
  +++ |+#include <functional>
   10 | 
/tmp/Vy2hKGxdRp/main.cpp:96:64: error: expression list treated as compound expression in functional cast [-fpermissive]
   96 |          function<void(int, int, vector<bool>&, vector<int>&)> backtrack;
      |                                                             ^

/tmp/Vy2hKGxdRp/main.cpp:96:22: error: expected primary-expression before 'void'
   96 |             function<void(int, int, vector<bool>&, vector<int>&)> backtrack;
      |                      ^~~~
ERROR!
/tmp/Vy2hKGxdRp/main.cpp:97:13: error: 'backtrack' was not declared in this scope
   97 |             backtrack = [&](int node, int current_dist, vector<bool>& local_vis, vector<int>& local_path) {
      |             ^~~~~~~~~


