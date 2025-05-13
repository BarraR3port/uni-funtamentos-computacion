package cl.unab.fundamentos.interfaces;

import java.util.List;

public interface BacktrackingProblem extends Cloneable {
    boolean isSolution();
    void applyMove(int move);
    void undoMove(int move);
    List<Integer> getPossibleMoves();
    Object getSolution();
    int getTargetSum();
    BacktrackingProblem clone();
    void printMatrix();
}
