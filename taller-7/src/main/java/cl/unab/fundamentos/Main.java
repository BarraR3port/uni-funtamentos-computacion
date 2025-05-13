package cl.unab.fundamentos;

import cl.unab.fundamentos.app.GraphBacktrackingProblem;
import cl.unab.fundamentos.app.SequentialGraphSolver;
import cl.unab.fundamentos.app.ParallelGraphSolver;
import cl.unab.fundamentos.utils.TimeLogger;
import java.util.*;

public class Main {
    private static final int INF = 9999;
    private static final int MAX_SIZE = 14;
    private static final int MIN_SIZE = 2;
    private static final int NUM_THREADS = 16;

    public static void main(String[] args) {
        System.out.println("Comparación de algoritmos secuencial vs paralelo para matrices desde 2x2 hasta 14x14");
        System.out.println("Mostrando el camino de menor costo encontrado\n");
        System.out.println("Configuración - Procesos disponibles: " + NUM_THREADS + "\n");

        Random rand = new Random();
        double totalSeqTime = 0;
        double totalParTime = 0;

        for (int n = MIN_SIZE; n <= MAX_SIZE; n++) {
            int[][] matrix = generarMatriz(n, rand);
            int start = rand.nextInt(n);
            int end = rand.nextInt(n);
            while (end == start) end = rand.nextInt(n);

            // Secuencial
            GraphBacktrackingProblem seqProblem = new GraphBacktrackingProblem(matrix, start, end);
            seqProblem.printMatrix();
            System.out.println("Punto inicial: " + start + ", Punto final: " + end + "\n");
            System.out.println("Resultados para matriz " + n + "x" + n + ":\n");

            long startSeq = System.nanoTime();
            SequentialGraphSolver seqSolver = new SequentialGraphSolver(seqProblem);
            seqSolver.solve();
            long endSeq = System.nanoTime();
            double tiempoSeq = (endSeq - startSeq) / 1000.0; // microsegundos
            totalSeqTime += tiempoSeq;

            System.out.println("[Secuencial]");
            System.out.println("Tiempo: " + tiempoSeq + " microsegundos");
            seqProblem.printPath();

            // Paralelo
            GraphBacktrackingProblem parProblem = new GraphBacktrackingProblem(matrix, start, end);
            long startPar = System.nanoTime();
            ParallelGraphSolver parSolver = new ParallelGraphSolver(parProblem, NUM_THREADS);
            parSolver.solve();
            long endPar = System.nanoTime();
            double tiempoPar = (endPar - startPar) / 1000.0; // microsegundos
            totalParTime += tiempoPar;

            System.out.println("\n[Paralelo]");
            System.out.println("Tiempo: " + tiempoPar + " microsegundos");
            System.out.println("Procesos utilizados: " + NUM_THREADS);
            parProblem.printPath();

            if (tiempoPar > 0) {
                System.out.println("\nSpeedup: " + (tiempoSeq / tiempoPar));
            }
            System.out.println("----------------------------------------\n");

            // Registrar tiempos en archivo
            TimeLogger.logExecutionTime(n, tiempoSeq, tiempoPar, NUM_THREADS);
        }

        // Registrar resumen final
        TimeLogger.logFinalSummary(MAX_SIZE, totalSeqTime, totalParTime);
    }

    private static int[][] generarMatriz(int n, Random rand) {
        int[][] matrix = new int[n][n];
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i == j) {
                    matrix[i][j] = 0;
                } else {
                    if (rand.nextDouble() < 0.05) {
                        matrix[i][j] = INF;
                    } else {
                        matrix[i][j] = 1 + rand.nextInt(10);
                    }
                }
            }
        }
        return matrix;
    }
}
