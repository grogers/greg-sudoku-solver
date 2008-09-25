#include "Sudoku.hpp"
#include "Logging.hpp"
#include "Techniques.hpp"

#include <boost/assign/list_of.hpp>

namespace {
bool SelectBifurcationCell(const Sudoku &, Index_t &row, Index_t &col);
const std::vector<Technique> bifurcationTechniques =
    boost::assign::list_of(&NakedSingle)(&HiddenSingle);
}

/**
 * Guess and Check.
 */
unsigned Bifurcate(Sudoku &sudoku)
{
    Log(Trace, "trying bifurcation\n");

    Index_t row, col, num;

    // if a cell cannot be found for bifurcation assume it cannot be solved
    if (!SelectBifurcationCell(sudoku, row, col))
        return 0;

    num = sudoku.GetCell(row, col).NumCandidates();
    Log(Info, "bifurcating on cell r%dc%d\n", row+1, col+1);

    std::vector<Sudoku> newSudokus(num, sudoku);
    unsigned numSolved = 0;
    const Sudoku *solved = NULL;

    /// @note this makes the solver non thread safe when doing a bifurcation,
    /// and is only to control the output due to the implementation of
    /// bifurcation. Remove it if you want.
    LogLevel oldLevel = QuietlyBifurcate();

    for (Index_t i = 1, idx = 0; i <= 9; ++i) {
        if (!sudoku.GetCell(row, col).IsCandidate(i))
            continue;

        Log(Trace, "trying bifurcation on cell r%dc%d of candidate %d\n", row+1, col+1, i);

        Cell cell = sudoku.GetCell(row, col);
        cell.SetValue(i);

        newSudokus[idx].SetCell(cell, row, col);
        newSudokus[idx].CrossHatch(row, col);

        unsigned tmp = newSudokus[idx].Solve(bifurcationTechniques, true);

        numSolved += tmp;

        if (tmp > 0)
            solved = &newSudokus[idx];

        if (tmp > 1)
            break;

        ++idx;
    }

    SetLogLevel(oldLevel);

    if (numSolved > 0)
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
