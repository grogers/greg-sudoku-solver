#include "Sudoku.hpp"
#include "Logging.hpp"
#include "Techniques.hpp"

bool SimpleSudokuTechniqueSet(Sudoku &sudoku)
{
    Log(Trace, "applying simple sudoku technique set\n");
    if (NakedSingle(sudoku))
        return true;
    if (HiddenSingle(sudoku))
        return true;
    if (NakedPair(sudoku))
        return true;
    if (LockedCandidates(sudoku))
        return true;
    if (NakedTriple(sudoku))
        return true;
    if (NakedQuad(sudoku))
        return true;
    if (HiddenPair(sudoku))
        return true;
    if (XWing(sudoku))
        return true;
    if (Swordfish(sudoku))
        return true;
    if (SimpleColor(sudoku))
        return true;
    if (MultiColor(sudoku))
        return true;
    if (HiddenTriple(sudoku))
        return true;
    if (XyWing(sudoku))
        return true;
    if (HiddenQuad(sudoku))
        return true;
    if (Jellyfish(sudoku))
        return true;

    return false;
}
