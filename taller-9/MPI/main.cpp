#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>

void slowSort(std::vector<double>& A, int i, int j) {
    if (i >= j) return;
    int m = (i + j) / 2;
    slowSort(A, i, m);
    slowSort(A, m + 1, j);
    if (A[j] < A[m]) std::swap(A[j], A[m]);
    slowSort(A, i, j - 1);
}

std::vector<double> ordenarDesdeMemoria(const char* archivo, int columna_objetivo, long long& duracion_ms) {
    int fd = open(archivo, O_RDONLY);
    if (fd == -1) {
        std::cerr << "Error al abrir el archivo" << std::endl;
        return {};
    }

    struct stat info_archivo;
    if (fstat(fd, &info_archivo) == -1) {
        std::cerr << "Error al obtener información del archivo" << std::endl;
        close(fd);
        return {};
    }
    size_t tam_archivo = info_archivo.st_size;

    void* mapeo = mmap(NULL, tam_archivo, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapeo == MAP_FAILED) {
        std::cerr << "Error al mapear el archivo en memoria" << std::endl;
        close(fd);
        return {};
    }

    char* contenido = static_cast<char*>(mapeo);
    std::string linea;
    std::stringstream buffer;
    std::vector<double> valores;
    bool esPrimeraLinea = true;

    

    for (size_t i = 0; i < tam_archivo; ++i) {
        if (contenido[i] != '\n') {
            buffer << contenido[i];
        } else {
            linea = buffer.str();
            buffer.str("");
            buffer.clear();

            if (esPrimeraLinea) {
                esPrimeraLinea = false;
                continue;
            }

            std::stringstream ss(linea);
            std::string campo;
            int columna_actual = 0;
            while (std::getline(ss, campo, ',')) {
                if (columna_actual == columna_objetivo) {
                    try {
                        valores.push_back(std::stod(campo));
                    } catch (...) {
                        std::cerr << "Error al convertir: " << linea << std::endl;
                    }
                    break;
                }
                columna_actual++;
            }
        }
    }
    auto inicio = std::chrono::high_resolution_clock::now();
    slowSort(valores, 0, valores.size() - 1);

    auto fin = std::chrono::high_resolution_clock::now();
    duracion_ms = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio).count();

    munmap(mapeo, tam_archivo);
    close(fd);
    return valores;
}

std::vector<double> ordenarDesdeArchivo(const char* archivo, int columna_objetivo, long long& duracion_ms) {
    std::ifstream infile(archivo);
    if (!infile.is_open()) {
        std::cerr << "Error al abrir archivo" << std::endl;
        return {};
    }

    std::string linea;
    std::vector<double> valores;
    bool esPrimeraLinea = true;

    

    while (std::getline(infile, linea)) {
        if (esPrimeraLinea) {
            esPrimeraLinea = false;
            continue;
        }

        std::stringstream ss(linea);
        std::string campo;
        int columna_actual = 0;
        while (std::getline(ss, campo, ',')) {
            if (columna_actual == columna_objetivo) {
                try {
                    valores.push_back(std::stod(campo));
                } catch (...) {
                    std::cerr << "Error al convertir: " << linea << std::endl;
                }
                break;
            }
            columna_actual++;
        }
    }

    infile.close();
    auto inicio = std::chrono::high_resolution_clock::now();
    slowSort(valores, 0, valores.size() - 1);

    auto fin = std::chrono::high_resolution_clock::now();
    duracion_ms = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio).count();

    return valores;
}

int main(int argc, char* argv[]) {

    const char* archivo = "ejemplo_500.csv";
    int columna = 1;
    long long tiempo_memoria = 0;
    long long tiempo_disco = 0;

    std::vector<double> datos_mem = ordenarDesdeMemoria(archivo, columna, tiempo_memoria);
    std::vector<double> datos_disco = ordenarDesdeArchivo(archivo, columna, tiempo_disco);

    std::cout << "----- SlowSort desde mmap -----\n";
    std::cout << "Tiempo: " << tiempo_memoria << " ms\n";
    std::cout << "Primeros 10 elementos ordenados:\n";
    for (size_t i = 0; i < std::min((size_t)10, datos_mem.size()); ++i)
        std::cout << datos_mem[i] << " ";
    std::cout << "\n\n";

    std::cout << "----- SlowSort desde archivo -----\n";
    std::cout << "Tiempo: " << tiempo_disco << " ms\n";
    std::cout << "Primeros 10 elementos ordenados:\n";
    for (size_t i = 0; i < std::min((size_t)10, datos_disco.size()); ++i)
        std::cout << datos_disco[i] << " ";
    std::cout << std::endl;

    return 0;
}
