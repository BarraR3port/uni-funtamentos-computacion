package cl.unab.fundamentos;

import cl.unab.fundamentos.utils.MatrixUtils;
import cl.unab.fundamentos.utils.TimeLogger;

public class Main {
    private static final int MAX_SIZE = 14;
    private static final int MIN_SIZE = 2;
    private static final int NUM_THREADS = 16;

    public static void main(String[] args) {
        StringBuilder output = new StringBuilder();
        output.append("Comparación de algoritmos secuencial vs paralelo para matrices desde 2x2 hasta 14x14\n");
        output.append("Mostrando el camino de menor costo encontrado\n\n");
        output.append("Configuración - Procesos disponibles: ").append(NUM_THREADS).append("\n\n");
        System.out.print(output.toString());

        double totalSeqTime = 0;
        double totalParTime = 0;

        for (int n = MIN_SIZE; n <= MAX_SIZE; n++) {
            MatrixUtils.ExecutionResult result = MatrixUtils.ejecutarProblema(n, NUM_THREADS);
            System.out.print(result.output);
            totalSeqTime += result.tiempoSecuencial;
            totalParTime += result.tiempoParalelo;
        }

        // Registrar resumen final
        TimeLogger.logFinalSummary(MAX_SIZE, totalSeqTime, totalParTime);
    }
}
