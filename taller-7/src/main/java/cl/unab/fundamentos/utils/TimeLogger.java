package cl.unab.fundamentos.utils;

import java.io.FileWriter;
import java.io.IOException;
import java.io.File;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

public class TimeLogger {
    private static final String LOG_DIR = "taller-7";
    private static final DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
    private static final DateTimeFormatter fileFormatter = DateTimeFormatter.ofPattern("yyyy-MM-dd_HH-mm-ss");
    private static final String LOG_FILE;
    private static final LocalDateTime startTime;

    static {
        // Capturar el tiempo de inicio
        startTime = LocalDateTime.now();
        
        // Crear el nombre del archivo con la fecha y hora
        String timestamp = startTime.format(fileFormatter);
        LOG_FILE = LOG_DIR + File.separator + "ex-" + timestamp + ".txt";

        // Crear el directorio si no existe
        File directory = new File(LOG_DIR);
        if (!directory.exists()) {
            directory.mkdirs();
        }
    }

    public static void logExecutionTime(int matrixSize, double seqTime, double parTime, int numThreads) {
        try (FileWriter writer = new FileWriter(LOG_FILE, true)) {
            StringBuilder sb = new StringBuilder();
            
            // Si el archivo está vacío, agregar encabezado
            if (new File(LOG_FILE).length() == 0) {
                sb.append("=== Registro de Tiempos de Ejecución ===\n");
                sb.append("Fecha de inicio: ").append(startTime.format(formatter)).append("\n");
                sb.append("Tamaño máximo de matrices: ").append(matrixSize).append("x").append(matrixSize).append("\n");
                sb.append("Número de threads: ").append(numThreads).append("\n");
                sb.append("============================================\n\n");
            }

            // Agregar entrada de tiempo
            sb.append("Matriz ").append(matrixSize).append("x").append(matrixSize).append(":\n");
            sb.append("Tiempo secuencial: ").append(String.format("%.2f", seqTime)).append(" microsegundos\n");
            sb.append("Tiempo paralelo: ").append(String.format("%.2f", parTime)).append(" microsegundos\n");
            if (parTime > 0) {
                sb.append("Speedup: ").append(String.format("%.4f", seqTime/parTime)).append("\n");
            }
            sb.append("----------------------------------------\n");

            writer.write(sb.toString());
        } catch (IOException e) {
            System.err.println("Error al escribir en el archivo de tiempos: " + e.getMessage());
        }
    }

    public static void logFinalSummary(int maxSize, double totalSeqTime, double totalParTime) {
        try (FileWriter writer = new FileWriter(LOG_FILE, true)) {
            StringBuilder sb = new StringBuilder();
            
            sb.append("\n=== Resumen Final ===\n");
            sb.append("Fecha de inicio: ").append(startTime.format(formatter)).append("\n");
            sb.append("Fecha de finalización: ").append(LocalDateTime.now().format(formatter)).append("\n");
            sb.append("Total matrices procesadas: ").append(maxSize - 1).append("\n");
            sb.append("Tiempo total secuencial: ").append(String.format("%.2f", totalSeqTime)).append(" microsegundos\n");
            sb.append("Tiempo total paralelo: ").append(String.format("%.2f", totalParTime)).append(" microsegundos\n");
            if (totalParTime > 0) {
                sb.append("Speedup promedio: ").append(String.format("%.4f", totalSeqTime/totalParTime)).append("\n");
            }
            sb.append("============================================\n\n");

            writer.write(sb.toString());
        } catch (IOException e) {
            System.err.println("Error al escribir el resumen final: " + e.getMessage());
        }
    }
} 