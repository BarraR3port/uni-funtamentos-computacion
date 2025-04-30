#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <mutex>
#include <climits>
#include <algorithm>
#include <iomanip>
#include <mpi.h>

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

// Función para serializar un camino
vector<int> serializarCamino(const Camino& camino) {
    vector<int> serializado;
    serializado.push_back(camino.costo);
    serializado.push_back(camino.nodos.size());
    serializado.insert(serializado.end(), camino.nodos.begin(), camino.nodos.end());
    return serializado;
}

// Función para deserializar un camino
Camino deserializarCamino(const vector<int>& serializado) {
    Camino camino;
    camino.costo = serializado[0];
    int size = serializado[1];
    camino.nodos.insert(camino.nodos.end(), serializado.begin() + 2, serializado.begin() + 2 + size);
    return camino;
}

// Backtracking paralelo con MPI
void backtracking_paralelo_mpi(vector<vector<int>>& matrix, int start, int end, 
                             Camino& mejor_camino_global, int rank, int size) {
    Camino mejor_camino_local;
    mejor_camino_local.costo = INT_MAX;
    
    // Dividir el trabajo entre los procesos
    int n = matrix.size();
    int chunk_size = n / size;
    int start_idx = rank * chunk_size;
    int end_idx = (rank == size - 1) ? n : (rank + 1) * chunk_size;
    
    vector<bool> visited(n, false);
    visited[start] = true;
    
    for(int i = start_idx; i < end_idx; i++) {
        if(matrix[start][i] != 0 && matrix[start][i] != INF) {
            vector<int> camino_actual;
            camino_actual.push_back(start);
            
            vector<bool> new_visited = visited;
            new_visited[i] = true;
            
            Camino mejor_camino_parcial;
            mejor_camino_parcial.costo = INT_MAX;
            
            backtracking_secuencial(matrix, i, end, matrix[start][i], 
                                   camino_actual, mejor_camino_parcial, new_visited);
            
            if(mejor_camino_parcial.costo < mejor_camino_local.costo) {
                mejor_camino_local = mejor_camino_parcial;
            }
        }
    }
    
    // Serializar el mejor camino local
    vector<int> serializado_local = serializarCamino(mejor_camino_local);
    int local_size = serializado_local.size();
    
    // Recolectar los tamaños de todos los procesos
    vector<int> all_sizes(size);
    MPI_Gather(&local_size, 1, MPI_INT, all_sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Preparar para recibir todos los caminos en el proceso 0
    if(rank == 0) {
        vector<int> all_data;
        vector<int> displacements(size, 0);
        
        // Calcular desplazamientos
        for(int i = 1; i < size; i++) {
            displacements[i] = displacements[i-1] + all_sizes[i-1];
        }
        int total_size = displacements.back() + all_sizes.back();
        all_data.resize(total_size);
        
        // Recolectar todos los datos
        MPI_Gatherv(serializado_local.data(), local_size, MPI_INT,
                   all_data.data(), all_sizes.data(), displacements.data(), MPI_INT,
                   0, MPI_COMM_WORLD);
        
        // Encontrar el mejor camino global
        mejor_camino_global.costo = INT_MAX;
        for(int i = 0; i < size; i++) {
            vector<int> chunk(all_data.begin() + displacements[i],
                             all_data.begin() + displacements[i] + all_sizes[i]);
            Camino camino = deserializarCamino(chunk);
            if(camino.costo < mejor_camino_global.costo) {
                mejor_camino_global = camino;
            }
        }
    } else {
        // Enviar datos al proceso 0
        MPI_Gatherv(serializado_local.data(), local_size, MPI_INT,
                   nullptr, nullptr, nullptr, MPI_INT,
                   0, MPI_COMM_WORLD);
    }
}

void medir_tiempos(int n, int rank, int size) {
    vector<vector<int>> matrix;
    int start, end;
    
    // Generar la matriz solo en el proceso 0 y luego difundirla
    if(rank == 0) {
        matrix = generarMatriz(n);
        mostrarMatriz(matrix);
        
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, n-1);
        
        start = dis(gen);
        end = dis(gen);
        while(start == end) {
            end = dis(gen);
        }
        
        cout << "Punto inicial: " << start << ", Punto final: " << end << endl;
    }
    
    // Difundir la matriz y los puntos de inicio/fin a todos los procesos
    MPI_Bcast(&start, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&end, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if(rank != 0) {
        matrix.resize(n, vector<int>(n));
    }
    for(int i = 0; i < n; i++) {
        MPI_Bcast(matrix[i].data(), n, MPI_INT, 0, MPI_COMM_WORLD);
    }
    
    // Configuración para secuencial (solo en proceso 0)
    Camino mejor_secuencial;
    double tiempo_seq = 0.0;
    
    if(rank == 0) {
        mejor_secuencial.costo = INT_MAX;
        vector<int> camino_actual_seq;
        vector<bool> visited_seq(n, false);
        visited_seq[start] = true;
        
        auto inicio_seq = high_resolution_clock::now();
        backtracking_secuencial(matrix, start, end, 0, camino_actual_seq, mejor_secuencial, visited_seq);
        auto fin_seq = high_resolution_clock::now();
        tiempo_seq = duration_cast<microseconds>(fin_seq - inicio_seq).count();
    }
    
    // Configuración para paralelo
    Camino mejor_paralelo;
    double tiempo_par = 0.0;
    
    MPI_Barrier(MPI_COMM_WORLD);
    auto inicio_par = high_resolution_clock::now();
    
    backtracking_paralelo_mpi(matrix, start, end, mejor_paralelo, rank, size);
    
    MPI_Barrier(MPI_COMM_WORLD);
    auto fin_par = high_resolution_clock::now();
    
    if(rank == 0) {
        tiempo_par = duration_cast<microseconds>(fin_par - inicio_par).count();
        
        // Mostrar resultados
        cout << "\nResultados para matriz " << n << "x" << n << ":" << endl;
        
        cout << "\n[Secuencial]" << endl;
        cout << "Tiempo: " << tiempo_seq << " microsegundos" << endl;
        mostrarCamino(mejor_secuencial, matrix);
        
        cout << "\n[Paralelo - MPI]" << endl;
        cout << "Tiempo: " << tiempo_par << " microsegundos" << endl;
        cout << "Procesos utilizados: " << size << endl;
        mostrarCamino(mejor_paralelo, matrix);
        
        if(tiempo_par > 0) {
            cout << "\nSpeedup: " << (double)tiempo_seq/tiempo_par << endl;
        }
        cout << "----------------------------------------\n" << endl;
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if(rank == 0) {
        cout << "Comparación de algoritmos secuencial vs paralelo (MPI) para matrices desde 2x2 hasta 20x20\n";
        cout << "Mostrando el camino de menor costo encontrado\n\n";
        cout << "Configuración MPI - Procesos disponibles: " << size << "\n\n";
    }
    
    for(int n = 2; n <= 14; n++) {
        medir_tiempos(n, rank, size);
    }
    
    MPI_Finalize();
    return 0;
}