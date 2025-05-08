package cl.unab.fundamentos;

import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicBoolean;
import cl.unab.fundamentos.app.MyBacktrackingProblem;
import cl.unab.fundamentos.app.ParallelBacktracking;
import cl.unab.fundamentos.interfaces.BacktrackingProblem;
public class Main {
    public static void main(String[] args) {
        BacktrackingProblem problem = new MyBacktrackingProblem(); // Reemplaza esto con la instancia de tu problema de backtracking
        AtomicBoolean foundSolution = new AtomicBoolean(false);
        ConcurrentLinkedQueue<Object> solutions = new ConcurrentLinkedQueue<>();
        List<Integer> initialMoves = problem.getPossibleMoves();
        List<Thread> threads = new ArrayList<>();
        System.out.println("Iniciando backtracking paralelo...");
        long startTime = System.currentTimeMillis();
        // Crea un thread para cada movimiento inicial posible
        for (int move : initialMoves) {
            MyBacktrackingProblem problemCopy = ((MyBacktrackingProblem) problem).clone(); // Create a copy for each thread
            problemCopy.applyMove(move);
            ParallelBacktracking parallelBacktracking = new ParallelBacktracking(problemCopy, foundSolution, solutions);
            Thread thread = new Thread(parallelBacktracking);
            threads.add(thread);
            thread.start();
        }
        // Espera a que todos los threads terminen
        for (Thread thread : threads) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        long endTime = System.currentTimeMillis();
        StringBuilder sb = new StringBuilder();
        sb.append("Backtracking paralelo completado en ").append(endTime - startTime).append(" ms");
        System.out.println(sb.toString());
        // Imprime las soluciones encontradas
        while (!solutions.isEmpty()) {
            System.out.println(solutions.poll());
        }
    }
}
