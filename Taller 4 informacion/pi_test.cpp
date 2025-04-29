#include <iostream>
#include <omp.h>
#include <chrono>
#include <cmath>

using namespace std;
using namespace std::chrono;

// Versión secuencial del cálculo de PI
double calcularPiSecuencial(int n) {
    double pi = 0.0;
    for (int i = 1; i <= n; i++) {
        pi += 4.0 / (i * 2 - 1) * ((i % 2 == 1) ? 1 : -1);
    }
    return pi;
}

// Versión paralela del cálculo de PI usando OpenMP
double calcularPiParalelo(int n) {
    double pi = 0.0;
    #pragma omp parallel for reduction(+:pi)
    for (int i = 1; i <= n; i++) {
        pi += 4.0 / (i * 2 - 1) * ((i % 2 == 1) ? 1 : -1);
    }
    return pi;
}

void ejecutarPrueba(int n) {
    cout << "Calculando PI con n = " << n << endl;
    
    // Versión secuencial
    auto inicio = high_resolution_clock::now();
    double pi_sec = calcularPiSecuencial(n);
    auto fin = high_resolution_clock::now();
    auto tiempo_sec = duration_cast<microseconds>(fin - inicio).count();
    
    // Versión paralela
    inicio = high_resolution_clock::now();
    double pi_par = calcularPiParalelo(n);
    fin = high_resolution_clock::now();
    auto tiempo_par = duration_cast<microseconds>(fin - inicio).count();
    
    // Mostrar resultados
    cout << "Secuencial: PI ≈ " << pi_sec << " - Tiempo: " << tiempo_sec << " μs" << endl;
    cout << "Paralelo:   PI ≈ " << pi_par << " - Tiempo: " << tiempo_par << " μs" << endl;
    cout << "Aceleración: " << (double)tiempo_sec/tiempo_par << "x" << endl;
    cout << "Diferencia entre resultados: " << fabs(pi_sec - pi_par) << endl << endl;
}

int main() {
    // Ejecutar pruebas para los diferentes valores de n
    ejecutarPrueba(1000);
    ejecutarPrueba(10000);
    ejecutarPrueba(3);
    
    return 0;
}