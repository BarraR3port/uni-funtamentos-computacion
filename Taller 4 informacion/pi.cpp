#include <iostream>
#include <chrono>
#include <omp.h>
#include <cmath>

using namespace std;
using namespace std::chrono;

// Función para calcular PI de forma secuencial
double calcularPiSecuencial(int n) {
    double pi = 0.0;
    for (int i = 1; i <= n; i++) {
        double termino = 4.0 / (i * 2 - 1);
        if (i % 2 == 0) {
            pi -= termino;
        } else {
            pi += termino;
        }
    }
    return pi;
}

// Función para calcular PI de forma paralela
double calcularPiParalelo(int n) {
    double pi = 0.0;
    #pragma omp parallel for reduction(+:pi)
    for (int i = 1; i <= n; i++) {
        double termino = 4.0 / (i * 2 - 1);
        if (i % 2 == 0) {
            pi -= termino;
        } else {
            pi += termino;
        }
    }
    return pi;
}

int main() {
    //int valores_n[] = {1000, 10000, 3};
    int n = 10000; // Cambia este valor para probar con diferentes tamaños de n
    int i;
    
    for (i=0; i <=n; i++) {
        cout << "\n--- Calculando PI para n = " << n << " ---" << endl;
        
        // Versión secuencial
        
        auto inicio_sec = high_resolution_clock::now();
        double pi_sec = calcularPiSecuencial(i);
        auto fin_sec = high_resolution_clock::now();
        auto tiempo_sec = duration_cast<microseconds>(fin_sec - inicio_sec).count();
        
        // Versión paralela
        auto inicio_par = high_resolution_clock::now();
        double pi_par = calcularPiParalelo(i);
        auto fin_par = high_resolution_clock::now();
        auto tiempo_par = duration_cast<microseconds>(fin_par - inicio_par).count();
        
        // Resultados
        //float tiempo_sec;
        //float tiempo_par;
        cout << "PI (secuencial): " << pi_sec << " - Tiempo: " << tiempo_sec << " μs" << endl;
        cout << "PI (paralelo):   " << pi_par << " - Tiempo: " << tiempo_par << " μs" << endl;
        cout << "Diferencia: " << fabs(pi_sec - pi_par) << endl;
        cout << "Mejora: " << (tiempo_sec - tiempo_par) * 100.0 / tiempo_sec << "% más rápido" << endl;
    }
    +
    
    return 0;
}