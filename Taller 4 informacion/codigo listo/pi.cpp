#include <iostream> 
#include <omp.h> 
#include <chrono> 
#include <cmath> 
#include <iomanip> 
  
using namespace std; 
using namespace std::chrono; 
  
// Versión secuencial del cálculo de PI 
double calcularPiSecuencial(long n) { 
    double pi = 0.0; 
    for (long i = 0; i < n; i++) { 
        double term = 1.0 / (2 * i + 1); 
        if (i % 2 == 0) { 
            pi += term; 
        } else { 
            pi -= term; 
        } 
    } 
    return pi * 4; 
} 
  
// Versión paralela del cálculo de PI usando OpenMP 
double calcularPiParalelo(long n) { 
    double pi = 0.0; 
    #pragma omp parallel for reduction(+:pi) 
    for (long i = 0; i < n; i++) { 
        double term = 1.0 / (2 * i + 1); 
        if (i % 2 == 0) { 
            pi += term; 
        } else { 
            pi -= term; 
        } 
    } 
    return pi * 4; 
} 
  
void ejecutarPrueba(long n) { 
    cout << "Calculando PI con n = " << n << endl; 
     
    // Versión secuencial 
    auto inicio_sec = high_resolution_clock::now(); 
    double pi_sec = calcularPiSecuencial(n); 
    auto fin_sec = high_resolution_clock::now(); 
    auto tiempo_sec = duration_cast<microseconds>(fin_sec - inicio_sec).count(); 
     
    // Versión paralela 
    auto inicio_par = high_resolution_clock::now(); 
    double pi_par = calcularPiParalelo(n); 
    auto fin_par = high_resolution_clock::now(); 
    auto tiempo_par = duration_cast<microseconds>(fin_par - inicio_par).count(); 
     
    // Calcular diferencia de tiempo y aceleración 
    auto diferencia_tiempo = tiempo_sec - tiempo_par; 
    double aceleracion = (tiempo_sec > 0) ? static_cast<double>(tiempo_sec)/tiempo_par : 0; 
     
    // Mostrar resultados 
    cout << fixed << setprecision(15); 
    cout << "Secuencial: PI ≈ " << pi_sec << " - Tiempo: " << tiempo_sec << " μs" << endl; 
    cout << "Paralelo:   PI ≈ " << pi_par << " - Tiempo: " << tiempo_par << " μs" << endl; 
    cout << "Diferencia de tiempo: " << diferencia_tiempo << " μs (secuencial - paralelo)" << endl; 
    cout << "Aceleración: " << aceleracion << "x" << endl; 
    cout << "Diferencia entre resultados: " << abs(pi_sec - pi_par) << endl << endl; 
} 
  
int main() { 
    // Configurar número de hilos (opcional) 
    // omp_set_num_threads(4); 
     
    // Ejecutar pruebas para los diferentes valores de n 
    ejecutarPrueba(1000); 
    ejecutarPrueba(10000); 
    ejecutarPrueba(3); 
     
    return 0; 
} 
 

