package cl.unab.fundamentos.app;

import cl.unab.fundamentos.interfaces.BacktrackingProblem;
import java.util.*;

public class GraphBacktrackingProblem implements BacktrackingProblem {
    private final int[][] matrix;
    private final int n;
    private final int start;
    private final int end;
    private List<Integer> bestPath;
    private int bestCost;
    private List<Integer> currentPath;
    private int currentCost;
    private boolean[] visited;
    public static final int INF = 9999;

    public GraphBacktrackingProblem(int[][] matrix, int start, int end) {
        this.matrix = matrix;
        this.n = matrix.length;
        this.start = start;
        this.end = end;
        this.bestPath = new ArrayList<>();
        this.bestCost = Integer.MAX_VALUE;
        this.currentPath = new ArrayList<>();
        this.currentCost = 0;
        this.visited = new boolean[n];
    }

    // Copy constructor
    public GraphBacktrackingProblem(GraphBacktrackingProblem other) {
        this.n = other.n;
        this.matrix = new int[n][n];
        for (int i = 0; i < n; i++) {
            System.arraycopy(other.matrix[i], 0, this.matrix[i], 0, n);
        }
        this.start = other.start;
        this.end = other.end;
        this.bestPath = new ArrayList<>(other.bestPath);
        this.bestCost = other.bestCost;
        this.currentPath = new ArrayList<>(other.currentPath);
        this.currentCost = other.currentCost;
        this.visited = Arrays.copyOf(other.visited, n);
    }

    @Override
    public boolean isSolution() {
        return !currentPath.isEmpty() && currentPath.get(currentPath.size() - 1) == end;
    }

    @Override
    public void applyMove(int move) {
        if (!currentPath.isEmpty()) {
            currentCost += matrix[currentPath.get(currentPath.size() - 1)][move];
        }
        currentPath.add(move);
        visited[move] = true;
    }

    @Override
    public void undoMove(int move) {
        if (currentPath.size() > 1) {
            int from = currentPath.get(currentPath.size() - 2);
            currentCost -= matrix[from][move];
        }
        visited[move] = false;
        currentPath.remove(currentPath.size() - 1);
    }

    @Override
    public List<Integer> getPossibleMoves() {
        List<Integer> moves = new ArrayList<>();
        int last = currentPath.isEmpty() ? start : currentPath.get(currentPath.size() - 1);
        for (int i = 0; i < n; i++) {
            if (!visited[i] && matrix[last][i] != 0 && matrix[last][i] != INF) {
                moves.add(i);
            }
        }
        return moves;
    }

    @Override
    public Object getSolution() {
        return new ArrayList<>(bestPath);
    }

    @Override
    public int getTargetSum() {
        return bestCost;
    }

    @Override
    public BacktrackingProblem clone() {
        return new GraphBacktrackingProblem(this);
    }

    @Override
    public void printMatrix() {
        int limit = Math.min(n, matrix.length);
        StringBuilder sb = new StringBuilder();
        
        // Add top border
        sb.append("┌");
        for (int j = 0; j < limit * 4 + (limit-1); j++) {
            sb.append("─");
        }
        sb.append("┐\n");

        // Add matrix content with side borders
        for (int i = 0; i < limit; i++) {
            sb.append("│");
            for (int j = 0; j < limit; j++) {
                if (matrix[i][j] == INF) {
                    sb.append(String.format("%4s", "INF"));
                } else {
                    sb.append(String.format("%4d", matrix[i][j]));
                }
                if (j < limit-1) sb.append(" ");
            }
            sb.append("│\n");
        }

        // Add bottom border
        sb.append("└");
        for (int j = 0; j < limit * 4 + (limit-1); j++) {
            sb.append("─");
        }
        sb.append("┘");

        // Print the complete matrix
        System.out.println(sb.toString());
    }

    public void printPath() {
        if (bestCost == Integer.MAX_VALUE) {
            System.out.println("No existe camino válido");
            return;
        }
        System.out.print("Camino con costo " + bestCost + ": ");
        for (int i = 0; i < bestPath.size(); i++) {
            System.out.print(bestPath.get(i));
            if (i < bestPath.size() - 1) {
                int from = bestPath.get(i);
                int to = bestPath.get(i + 1);
                int cost = matrix[from][to];
                System.out.print(" -(" + (cost == INF ? "INF" : cost) + ")-> ");
            }
        }
        System.out.println();
    }

    // Métodos públicos para los solvers
    public void addToPath(int node) {
        if (!currentPath.isEmpty()) {
            currentCost += matrix[currentPath.get(currentPath.size() - 1)][node];
        }
        currentPath.add(node);
    }

    public void removeFromPath() {
        if (currentPath.size() > 1) {
            int last = currentPath.get(currentPath.size() - 1);
            int prev = currentPath.get(currentPath.size() - 2);
            currentCost -= matrix[prev][last];
        }
        currentPath.remove(currentPath.size() - 1);
    }

    public void setBestPath(List<Integer> path) {
        this.bestPath = new ArrayList<>(path);
    }

    public void setBestCost(int cost) {
        this.bestCost = cost;
    }

    public List<Integer> getCurrentPath() {
        return new ArrayList<>(currentPath);
    }

    public int getStart() { return start; }
    public int getEnd() { return end; }
    public int[][] getMatrix() { return matrix; }
    public int getN() { return n; }
    public int getBestCost() { return bestCost; }
    public List<Integer> getBestPath() { return bestPath; }
} 