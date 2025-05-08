package cl.unab.fundamentos.app;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicBoolean;

import cl.unab.fundamentos.interfaces.BacktrackingProblem;

public class ParallelBacktracking implements Runnable {
    private BacktrackingProblem problem;
    private AtomicBoolean foundSolution;
    private ConcurrentLinkedQueue<Object> solutions;
    public ParallelBacktracking(BacktrackingProblem problem, AtomicBoolean foundSolution, ConcurrentLinkedQueue<Object> solutions) {
        this.problem = problem;
        this.foundSolution = foundSolution;
        this.solutions = solutions;
    }
    @Override
    public void run() {
        if (foundSolution.get()) {
            return;
        }
        if (problem.isSolution()) {
            solutions.add(problem.getSolution());
            foundSolution.set(true);
            return;
        }
        List<Integer> moves = problem.getPossibleMoves();
        for (int move : moves) {
            if (foundSolution.get()) {
                return;
            }
            problem.applyMove(move);
            ParallelBacktracking child = new ParallelBacktracking(problem, foundSolution, solutions);
            child.run();
            problem.undoMove(move);
        }
    }
    public BacktrackingProblem getBacktrackingProblem(){
        return this.problem;
    }
    public void setBacktrackingProblem(BacktrackingProblem problem){
        this.problem = problem;
    }
}
