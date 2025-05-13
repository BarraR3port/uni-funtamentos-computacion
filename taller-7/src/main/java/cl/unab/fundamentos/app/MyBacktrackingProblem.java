package cl.unab.fundamentos.app;

import java.util.List;
import java.util.ArrayList;
import cl.unab.fundamentos.interfaces.BacktrackingProblem;

public class MyBacktrackingProblem implements BacktrackingProblem{
    private List<Integer> currentState = new ArrayList<>();
    private final int targetSum;
    private final List<Integer> availableMoves;

    public MyBacktrackingProblem(int targetSum) {
        this.targetSum = targetSum;
        this.availableMoves = new ArrayList<>();
        for (int i = 1; i <= targetSum; i++) {
            availableMoves.add(i);
        }
    }

    // Copy constructor
    public MyBacktrackingProblem(MyBacktrackingProblem other) {
        this.currentState = new ArrayList<>(other.currentState);
        this.targetSum = other.targetSum;
        this.availableMoves = new ArrayList<>(other.availableMoves);
    }

    public MyBacktrackingProblem clone() {
        // Returns a new instance with the same state
        return new MyBacktrackingProblem(this);
    }

    @Override 
    public boolean isSolution() {
        int sum = currentState.stream().mapToInt(Integer::intValue).sum();
        return sum == targetSum;
    }

    @Override
    public void applyMove(int move){
        currentState.add(move);
    }

    @Override
    public void undoMove(int move){
        if (!currentState.isEmpty() && currentState.get(currentState.size() - 1 ) == move){
            currentState.remove(currentState.size() -1);
        }
    }
    @Override
    public List<Integer> getPossibleMoves(){
        return availableMoves;
    }

    @Override
    public Object getSolution() {
        return new ArrayList<>(currentState);
    }

    @Override
    public int getTargetSum() {
        return targetSum;
    }

    @Override
    public void printMatrix() {
        // Print the solution visually (each row = value in solution)
        System.out.println("Solution matrix (each row is a value in the solution):");
        for (int i = 0; i < currentState.size(); i++) {
            System.out.print("Row " + (i+1) + ": "); // Print row index
            for (int j = 0; j < currentState.get(i); j++) {
                System.out.print("* ");
            }
            System.out.println(); // New line for each row
        }
        System.out.println(); // Extra line for separation
    }

    // Método para obtener la solución como lista (útil para formatear)
    public List<Integer> getCurrentState() {
        return new ArrayList<>(currentState);
    }
}