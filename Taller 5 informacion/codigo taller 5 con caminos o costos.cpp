#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <mutex>
#include <climits>
#include <algorithm>
#include <iomanip>
#include <omp.h>

using namespace std;
using namespace std::chrono;

const int INF = 9999;
mutex mtx;

// Estructura para almacenar el camino
struct Camino {
    vector<int> nodos;
    int costo;
};

// Función para generar matriz de costos con posibles valores INF
vector<vector<int>> generarMatriz(int n) {
    vector<vector<int>> matrix(n, vector<int>(n));
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 10);
    uniform_real_distribution<> prob(0.0, 1.0);

    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            if(i == j) {
                matrix[i][j] = 0;
            } else {
                if(prob(gen) < 0.05) {
                    matrix[i][j] = INF;
                } else {
                    matrix[i][j] = dis(gen);
                }
            }
        }
    }
    return matrix;
}

void mostrarMatriz(const vector<vector<int>>& matrix) {
    int n = matrix.size();
    cout << "Matriz " << n << "x" << n << " (mostrando esquina 5x5 si n > 5):\n";
    
    int limit = min(n, 5);
    for(int i = 0; i < limit; i++) {
        for(int j = 0; j < limit; j++) {
            if(matrix[i][j] == INF) {
                cout << setw(4) << "INF";
            } else {
                cout << setw(4) << matrix[i][j];
            }
        }
        if(n > 5) cout << " ...";
        cout << endl;
    }
    if(n > 5) cout << "...\n";
}

// Backtracking secuencial con registro del camino
void backtracking_secuencial(vector<vector<int>>& matrix, int current, int end, 
                           int dist, vector<int>& camino_actual, Camino& mejor_camino, 
                           vector<bool>& visited) {
    camino_actual.push_back(current);
    
    if(current == end) {
        if(dist < mejor_camino.costo) {
            mejor_camino.costo = dist;
            mejor_camino.nodos = camino_actual;
        }
        camino_actual.pop_back();
        return;
    }

    for(int i = 0; i < matrix.size(); i++) {
        if(matrix[current][i] != 0 && matrix[current][i] != INF && !visited[i]) {
            visited[i] = true;
            backtracking_secuencial(matrix, i, end, dist + matrix[current][i], 
                                  camino_actual, mejor_camino, visited);
            visited[i] = false;
        }
    }
    camino_actual.pop_back();
}

// Backtracking paralelo con OpenMP y registro del camino
void backtracking_paralelo_omp(vector<vector<int>>& matrix, int current, int end, 
                             int dist, vector<int>& camino_actual, Camino& mejor_camino,
                             vector<bool>& visited, int depth = 0) {
    camino_actual.push_back(current);
    
    if(current == end) {
        #pragma omp critical
        {
            if(dist < mejor_camino.costo) {
                mejor_camino.costo = dist;
                mejor_camino.nodos = camino_actual;
            }
        }
        camino_actual.pop_back();
        return;
    }

    if(depth < 2) {
        #pragma omp parallel for shared(mejor_camino) firstprivate(visited, dist, camino_actual) schedule(dynamic)
        for(int i = 0; i < matrix.size(); i++) {
            if(matrix[current][i] != 0 && matrix[current][i] != INF && !visited[i]) {
                vector<bool> new_visited = visited;
                new_visited[i] = true;
                vector<int> new_camino = camino_actual;
                backtracking_paralelo_omp(matrix, i, end, dist + matrix[current][i], 
                                        new_camino, mejor_camino, new_visited, depth + 1);
            }
        }
    } else {
        for(int i = 0; i < matrix.size(); i++) {
            if(matrix[current][i] != 0 && matrix[current][i] != INF && !visited[i]) {
                visited[i] = true;
                backtracking_paralelo_omp(matrix, i, end, dist + matrix[current][i], 
                                        camino_actual, mejor_camino, visited, depth + 1);
                visited[i] = false;
            }
        }
    }
    camino_actual.pop_back();
}

// Función para mostrar el camino
void mostrarCamino(const Camino& camino, const vector<vector<int>>& matrix) {
    if(camino.costo == INT_MAX) {
        cout << "No existe camino válido\n";
        return;
    }
    
    cout << "Camino con costo " << camino.costo << ": ";
    for(size_t i = 0; i < camino.nodos.size(); i++) {
        cout << camino.nodos[i];
        if(i < camino.nodos.size() - 1) {
            int costo_arista = matrix[camino.nodos[i]][camino.nodos[i+1]];
            cout << " -(" << (costo_arista == INF ? "INF" : to_string(costo_arista)) << ")-> ";
        }
    }
    cout << endl;
}

void medir_tiempos(int n) {
    auto matrix = generarMatriz(n);
    mostrarMatriz(matrix);
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, n-1);
    
    int start = dis(gen);
    int end = dis(gen);
    while(start == end) {
        end = dis(gen);
    }
    
    cout << "Punto inicial: " << start << ", Punto final: " << end << endl;
    
    // Configuración para secuencial
    Camino mejor_secuencial;
    mejor_secuencial.costo = INT_MAX;
    vector<int> camino_actual_seq;
    vector<bool> visited_seq(n, false);
    visited_seq[start] = true;
    
    auto inicio_seq = high_resolution_clock::now();
    backtracking_secuencial(matrix, start, end, 0, camino_actual_seq, mejor_secuencial, visited_seq);
    auto fin_seq = high_resolution_clock::now();
    auto tiempo_seq = duration_cast<milliseconds>(fin_seq - inicio_seq).count();
    
    // Configuración para paralelo
    Camino mejor_paralelo;
    mejor_paralelo.costo = INT_MAX;
    vector<int> camino_actual_par;
    vector<bool> visited_par(n, false);
    visited_par[start] = true;
    
    auto inicio_par = high_resolution_clock::now();
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            backtracking_paralelo_omp(matrix, start, end, 0, camino_actual_par, mejor_paralelo, visited_par);
        }
    }
    auto fin_par = high_resolution_clock::now();
    auto tiempo_par = duration_cast<milliseconds>(fin_par - inicio_par).count();
    
    // Mostrar resultados
    cout << "\nResultados para matriz " << n << "x" << n << ":" << endl;
    
    cout << "\n[Secuencial]" << endl;
    cout << "Tiempo: " << tiempo_seq << " ms" << endl;
    mostrarCamino(mejor_secuencial, matrix);
    
    cout << "\n[Paralelo - OpenMP]" << endl;
    cout << "Tiempo: " << tiempo_par << " ms" << endl;
    cout << "Hilos utilizados: " << omp_get_max_threads() << endl;
    mostrarCamino(mejor_paralelo, matrix);
    
    if(tiempo_par > 0) {
        cout << "\nSpeedup: " << (double)tiempo_seq/tiempo_par << endl;
    }
    cout << "----------------------------------------\n" << endl;
}

int main() {
    cout << "Comparación de algoritmos secuencial vs paralelo (OpenMP) para matrices desde 2x2 hasta 20x20\n";
    cout << "Mostrando el camino de menor costo encontrado\n\n";
    
    omp_set_num_threads(omp_get_max_threads());
    cout << "Configuración OpenMP - Hilos disponibles: " << omp_get_max_threads() << "\n\n";
    
    for(int n = 2; n <= 20; n++) {
        medir_tiempos(n);
    }
    
    return 0;
}