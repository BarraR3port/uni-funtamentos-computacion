package cl.unab.fundamentos.app;

import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.ConcurrentLinkedQueue;

public class ParallelGraphSolver {
    private final GraphBacktrackingProblem problem;
    private final int numThreads;
    private final AtomicInteger bestCost;
    private final ConcurrentLinkedQueue<List<Integer>> bestPaths;

    public ParallelGraphSolver(GraphBacktrackingProblem problem, int numThreads) {
        this.problem = problem;
        this.numThreads = numThreads;
        this.bestCost = new AtomicInteger(Integer.MAX_VALUE);
        this.bestPaths = new ConcurrentLinkedQueue<>();
    }

    public void solve() {
        List<Thread> threads = new ArrayList<>();
        List<Integer> initialMoves = problem.getPossibleMoves();
        int movesPerThread = Math.max(1, initialMoves.size() / numThreads);

        for (int i = 0; i < numThreads && i * movesPerThread < initialMoves.size(); i++) {
            final int threadId = i;
            Thread thread = new Thread(() -> {
                int start = threadId * movesPerThread;
                int end = Math.min((threadId + 1) * movesPerThread, initialMoves.size());
                
                for (int moveIndex = start; moveIndex < end; moveIndex++) {
                    GraphBacktrackingProblem threadProblem = new GraphBacktrackingProblem(problem);
                    boolean[] visited = new boolean[threadProblem.getN()];
                    visited[threadProblem.getStart()] = true;
                    int move = initialMoves.get(moveIndex);
                    visited[move] = true;
                    threadProblem.addToPath(move);
                    backtrack(threadProblem, move, visited, threadProblem.getMatrix()[threadProblem.getStart()][move]);
                }
            });
            threads.add(thread);
            thread.start();
        }

        for (Thread thread : threads) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        // Update the original problem with the best solution found
        if (!bestPaths.isEmpty()) {
            List<Integer> bestPath = bestPaths.peek();
            problem.setBestPath(bestPath);
            problem.setBestCost(bestCost.get());
        }
    }

    private void backtrack(GraphBacktrackingProblem threadProblem, int node, boolean[] visited, int currentCost) {
        if (currentCost >= bestCost.get()) {
            return; // Early pruning if we found a better solution
        }

        if (node == threadProblem.getEnd()) {
            int oldCost = bestCost.get();
            if (currentCost < oldCost && bestCost.compareAndSet(oldCost, currentCost)) {
                bestPaths.offer(new ArrayList<>(threadProblem.getCurrentPath()));
            }
            return;
        }

        for (int next = 0; next < threadProblem.getN(); next++) {
            if (!visited[next] && threadProblem.getMatrix()[node][next] != 0 && 
                threadProblem.getMatrix()[node][next] != GraphBacktrackingProblem.INF) {
                visited[next] = true;
                threadProblem.addToPath(next);
                backtrack(threadProblem, next, visited, currentCost + threadProblem.getMatrix()[node][next]);
                threadProblem.removeFromPath();
                visited[next] = false;
            }
        }
    }
} 