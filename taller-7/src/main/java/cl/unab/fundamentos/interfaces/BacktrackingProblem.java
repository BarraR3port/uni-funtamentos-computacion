package cl.unab.fundamentos.interfaces;

import java.util.List;

public interface BacktrackingProblem {
    boolean isSolution();
    void applyMove(int move);
    void undoMove(int move);
    List<Integer> getPossibleMoves();
    Object getSolution();
}
