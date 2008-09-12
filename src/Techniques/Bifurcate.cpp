#include "Sudoku.hpp"
#include "Logging.hpp"

namespace {
bool SelectBifurcationCell(const Sudoku &, Index_t &row, Index_t &col);
}

/**
 * Guess and Check.
 */
unsigned Bifurcate(Sudoku &sudoku, const std::vector<Technique> &techniques)
{
    StarryLog(Trace, 3, "bifurcating... The output will now be for multiple puzzles, not just the one.\n");

    Index_t row, col, num;

    // if a cell cannot be found for bifurcation assume it cannot be solved
    if (!SelectBifurcationCell(sudoku, row, col))
        return 0;

    num = sudoku.GetCell(row, col).NumCandidates();
    Log(Debug, "bifurcating on cell (%d,%d) with %d candidates\n", row+1, col+1, num);

    std::vector<Sudoku> newSudokus(num, sudoku);
    unsigned numSolved = 0;
    const Sudoku *solved = NULL;

    for (Index_t i = 1, idx = 0; i <= 9; ++i) {
        if (!sudoku.GetCell(row, col).IsCandidate(i))
            continue;

        Log(Trace, "trying bifurcation on cell (%d,%d) of candidate %d\n", row+1, col+1, i);

        Cell cell = sudoku.GetCell(row, col);
        cell.SetValue(i);

        newSudokus[idx].SetCell(cell, row, col);
        newSudokus[idx].CrossHatch(row, col);

        unsigned tmp = newSudokus[idx].Solve(techniques);

        numSolved += tmp;

        if (tmp == 1)
            solved = &newSudokus[idx];

        ++idx;
    }

    if (numSolved == 1)
        sudoku = *solved;

    return numSolved;
}


namespace {

bool SelectBifurcationCell(const Sudoku &sudoku, Index_t &row, Index_t &col)
{
    for (Index_t n = 1; n <= 9; ++n) {
        for (Index_t i = 0; i < 9; ++i) {
            for (Index_t j = 0; j < 9; ++j) {
                if (sudoku.GetCell(i, j).NumCandidates() == n) {
                    row = i;
                    col = j;
                    return true;
                }
            }
        }
    }
    return false;
}

}
