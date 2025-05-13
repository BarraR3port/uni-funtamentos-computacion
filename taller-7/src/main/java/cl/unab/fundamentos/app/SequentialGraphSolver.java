package cl.unab.fundamentos.app;

public class SequentialGraphSolver {
    private final GraphBacktrackingProblem problem;

    public SequentialGraphSolver(GraphBacktrackingProblem problem) {
        this.problem = problem;
    }

    public void solve() {
        boolean[] visited = new boolean[problem.getN()];
        visited[problem.getStart()] = true;
        backtrack(problem.getStart(), visited, 0);
    }

    private void backtrack(int node, boolean[] visited, int currentCost) {
        if (node == problem.getEnd()) {
            if (currentCost < problem.getBestCost()) {
                problem.setBestCost(currentCost);
                problem.setBestPath(problem.getCurrentPath());
            }
            return;
        }

        for (int next = 0; next < problem.getN(); next++) {
            if (!visited[next] && problem.getMatrix()[node][next] != 0 && problem.getMatrix()[node][next] != GraphBacktrackingProblem.INF) {
                visited[next] = true;
                problem.addToPath(next);
                backtrack(next, visited, currentCost + problem.getMatrix()[node][next]);
                problem.removeFromPath();
                visited[next] = false;
            }
        }
    }
} 