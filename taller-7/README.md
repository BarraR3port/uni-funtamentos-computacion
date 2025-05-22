# Taller 7: Optimización de Matriz de Costos con Backtracking Paralelo en Java

Este proyecto implementa y compara algoritmos de backtracking secuencial y paralelo (usando threads) para encontrar el camino de menor costo en una matriz de adyacencia (grafo). El objetivo es analizar el impacto de la paralelización en problemas combinatorios y medir el speedup obtenido al distribuir el trabajo entre varios hilos.

## Estructura del Código
- **src/main/java/cl/unab/fundamentos/app/**: Contiene las clases principales del problema:
  - `GraphBacktrackingProblem.java`: Modela el problema del camino más corto.
  - `SequentialGraphSolver.java`: Implementa el algoritmo de backtracking secuencial.
  - `ParallelGraphSolver.java`: Implementa el algoritmo de backtracking paralelo usando threads.
- **src/main/java/cl/unab/fundamentos/utils/**: Utilidades para generación de matrices, registro de tiempos y orquestación de pruebas.
- **src/main/java/cl/unab/fundamentos/Main.java**: Punto de entrada del programa. Ejecuta los experimentos para diferentes tamaños de matriz y registra los resultados.

## Ejecución del Proyecto

### Requisitos
- Java 11 o superior instalado en el sistema.

### Compilación y Ejecución en Linux/Mac
1. Abre una terminal y navega al directorio `taller-7`.
2. Da permisos de ejecución al script (solo la primera vez):

   ```bash
   chmod +x start.sh
   ```

3. Ejecuta el script de inicio:

   ```bash
   ./start.sh
   ```

Este script compila automáticamente todos los archivos fuente Java y ejecuta la aplicación principal. Los resultados de la ejecución, incluyendo comparativas de tiempo y caminos óptimos encontrados, se mostrarán en la consola. Además, se generará un archivo de log en la carpeta `taller-7` con los detalles de cada ejecución.

### Alternativa para Windows
Si usas Windows y no puedes ejecutar `start.sh`, puedes compilar y ejecutar manualmente con los siguientes comandos en la terminal de Windows (CMD o PowerShell):

```bat
javac -d target/classes src/main/java/cl/unab/fundamentos/*.java src/main/java/cl/unab/fundamentos/*/*.java
java -cp target/classes cl.unab.fundamentos.Main
```

## Informe
El informe detallado del análisis, resultados y conclusiones se encuentra en el archivo [`analisis_codigo_taller7.tex`](analisis_codigo_taller7.tex). Este documento incluye:
- Explicación de la lógica y estructura del código.
- Comparación de los algoritmos secuencial y paralelo.
- Resultados experimentales y gráficos.
- Conclusiones sobre la eficiencia y escalabilidad de la paralelización.

---
**Autores:** Cristian Bravo, Bruno Bastidas, Silvio Villagra, Daniella Lecanda  
**Grupo investigativo Los Catalizadores** 