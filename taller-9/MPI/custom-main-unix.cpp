#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

void slowSort(std::vector<double>& A, int i, int j) {
    if (i >= j) return;
    int m = (i + j) / 2;
    slowSort(A, i, m);
    slowSort(A, m + 1, j);
    if (A[j] < A[m]) std::swap(A[j], A[m]);
    slowSort(A, i, j - 1);
}

#ifdef _WIN32
std::vector<double> ordenarDesdeMemoria(const char* archivo, int columna_objetivo, long long& duracion_ms) {
    HANDLE hFile = CreateFileA(archivo, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Error al abrir el archivo: " << GetLastError() << std::endl;
        return {};
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        std::cerr << "Error al obtener tamaño del archivo" << std::endl;
        CloseHandle(hFile);
        return {};
    }

    HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMapping == NULL) {
        std::cerr << "Error al crear mapeo de archivo: " << GetLastError() << std::endl;
        CloseHandle(hFile);
        return {};
    }

    LPVOID mappedData = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (mappedData == NULL) {
        std::cerr << "Error al mapear vista del archivo: " << GetLastError() << std::endl;
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return {};
    }

    char* contenido = static_cast<char*>(mappedData);
    std::string linea;
    std::stringstream buffer;
    std::vector<double> valores;
    bool esPrimeraLinea = true;

    for (DWORD i = 0; i < fileSize; ++i) {
        if (contenido[i] != '\n' && contenido[i] != '\r') {
            buffer << contenido[i];
        } else {
            if (contenido[i] == '\n') {
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
    }

    // Procesar la última línea si no terminó con salto de línea
    if (!buffer.str().empty()) {
        linea = buffer.str();
        if (!esPrimeraLinea) {
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

    UnmapViewOfFile(mappedData);
    CloseHandle(hMapping);
    CloseHandle(hFile);

    return valores;
}
#elif defined(__unix__) || defined(__APPLE__)
std::vector<double> ordenarDesdeMemoria(const char* archivo, int columna_objetivo, long long& duracion_ms) {
    int fd = open(archivo, O_RDONLY);
    if (fd == -1) {
        std::cerr << "Error al abrir el archivo" << std::endl;
        return {};
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        std::cerr << "Error al obtener tamaño del archivo" << std::endl;
        close(fd);
        return {};
    }
    size_t fileSize = sb.st_size;
    if (fileSize == 0) {
        close(fd);
        return {};
    }

    void* mapped = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        std::cerr << "Error al mapear archivo" << std::endl;
        close(fd);
        return {};
    }

    char* contenido = static_cast<char*>(mapped);
    std::string linea;
    std::stringstream buffer;
    std::vector<double> valores;
    bool esPrimeraLinea = true;

    for (size_t i = 0; i < fileSize; ++i) {
        if (contenido[i] != '\n' && contenido[i] != '\r') {
            buffer << contenido[i];
        } else {
            if (contenido[i] == '\n') {
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
    }

    // Procesar la última línea si no terminó con salto de línea
    if (!buffer.str().empty()) {
        linea = buffer.str();
        if (!esPrimeraLinea) {
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

    munmap(mapped, fileSize);
    close(fd);

    return valores;
}
#else
// Fallback: use file-based implementation
std::vector<double> ordenarDesdeMemoria(const char* archivo, int columna_objetivo, long long& duracion_ms) {
    return ordenarDesdeArchivo(archivo, columna_objetivo, duracion_ms);
}
#endif

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

int main() {
    const char* archivo = "ejemplo_500.csv";
    int columna = 1;
    long long tiempo_memoria = 0;
    long long tiempo_disco = 0;

    std::vector<double> datos_mem = ordenarDesdeMemoria(archivo, columna, tiempo_memoria);
    std::vector<double> datos_disco = ordenarDesdeArchivo(archivo, columna, tiempo_disco);

    std::cout << "----- SlowSort desde memoria (Windows API) -----\n";
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