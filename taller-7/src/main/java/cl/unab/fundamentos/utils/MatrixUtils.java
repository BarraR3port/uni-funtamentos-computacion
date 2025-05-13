package cl.unab.fundamentos.utils;

import cl.unab.fundamentos.app.GraphBacktrackingProblem;
import cl.unab.fundamentos.app.SequentialGraphSolver;
import cl.unab.fundamentos.app.ParallelGraphSolver;
import java.util.Random;

public class MatrixUtils {
    public static final int INF = 9999;

    public static int[][] generarMatriz(int n, Random rand) {
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

    public static ExecutionResult ejecutarProblema(int n, int numThreads) {
        Random rand = new Random();
        int[][] matrix = generarMatriz(n, rand);
        int start = rand.nextInt(n);
        int end = rand.nextInt(n);
        while (end == start) end = rand.nextInt(n);

        StringBuilder output = new StringBuilder();
        
        // Secuencial
        GraphBacktrackingProblem seqProblem = new GraphBacktrackingProblem(matrix, start, end);
        output.append(formatearMatriz(seqProblem));
        output.append("Punto inicial: ").append(start).append(", Punto final: ").append(end).append("\n\n");
        output.append("Resultados para matriz ").append(n).append("x").append(n).append(":\n\n");

        long startSeq = System.nanoTime();
        SequentialGraphSolver seqSolver = new SequentialGraphSolver(seqProblem);
        seqSolver.solve();
        long endSeq = System.nanoTime();
        double tiempoSeq = (endSeq - startSeq) / 1000.0;

        output.append("[Secuencial]\n");
        output.append("Tiempo: ").append(tiempoSeq).append(" microsegundos\n");
        output.append(formatearCamino(seqProblem));

        // Paralelo
        GraphBacktrackingProblem parProblem = new GraphBacktrackingProblem(matrix, start, end);
        long startPar = System.nanoTime();
        ParallelGraphSolver parSolver = new ParallelGraphSolver(parProblem, numThreads);
        parSolver.solve();
        long endPar = System.nanoTime();
        double tiempoPar = (endPar - startPar) / 1000.0;

        output.append("\n[Paralelo]\n");
        output.append("Tiempo: ").append(tiempoPar).append(" microsegundos\n");
        output.append("Procesos utilizados: ").append(numThreads).append("\n");
        output.append(formatearCamino(parProblem));

        if (tiempoPar > 0) {
            output.append("\nSpeedup: ").append(tiempoSeq / tiempoPar).append("\n");
        }
        output.append("----------------------------------------\n\n");

        // Registrar tiempos
        TimeLogger.logExecutionTime(n, tiempoSeq, tiempoPar, numThreads);

        return new ExecutionResult(output.toString(), tiempoSeq, tiempoPar);
    }

    private static String formatearMatriz(GraphBacktrackingProblem problem) {
        StringBuilder sb = new StringBuilder();
        problem.printMatrix();
        return sb.toString();
    }

    private static String formatearCamino(GraphBacktrackingProblem problem) {
        StringBuilder sb = new StringBuilder();
        problem.printPath();
        return sb.toString();
    }

    public static class ExecutionResult {
        public final String output;
        public final double tiempoSecuencial;
        public final double tiempoParalelo;

        public ExecutionResult(String output, double tiempoSecuencial, double tiempoParalelo) {
            this.output = output;
            this.tiempoSecuencial = tiempoSecuencial;
            this.tiempoParalelo = tiempoParalelo;
        }
    }
} 