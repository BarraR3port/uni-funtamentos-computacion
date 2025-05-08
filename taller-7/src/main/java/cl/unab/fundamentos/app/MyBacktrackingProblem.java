package cl.unab.fundamentos.app;

import java.util.List;
import java.util.ArrayList;
import cl.unab.fundamentos.interfaces.BacktrackingProblem;

public class MyBacktrackingProblem implements BacktrackingProblem{
    private List<Integer> currentState = new ArrayList<>();
    private final int targetSum = 14;
    private final List<Integer> availableMoves = List.of(1,2,3,4,5);

    public MyBacktrackingProblem() {
        // Default constructor
    }

    // Copy constructor
    public MyBacktrackingProblem(MyBacktrackingProblem other) {
        // Copy the current state
        this.currentState = new ArrayList<>(other.currentState);
        // Copy the target sum (it's final, but for clarity)
        // Copy the available moves (it's immutable, so safe to share)
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

}