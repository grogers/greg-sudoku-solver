#include "Sudoku.hpp"
#include "Logging.hpp"

namespace {
bool NakedSingleInCell(Cell &);
}

/**
 * Only 1 possible candidate in a cell.
 */
bool NakedSingle(Sudoku &sudoku)
{
    Log(Trace, "searching for naked singles\n");
    bool ret = false;

    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            Cell cell = sudoku.GetCell(i, j);
            if (NakedSingleInCell(cell)) {
                Log(Info, "naked single ==> r%dc%d = %d\n", i+1, j+1, cell.GetValue());
                sudoku.SetCell(cell, i, j);
                sudoku.CrossHatch(i, j);
                ret = true; // optimization - keep looping until all are found
            }
        }
    }
    return ret;
}

namespace {

/**
 * @return true if a change was found.
 */
bool NakedSingleInCell(Cell &cell)
{
    if (cell.NumCandidates() == 1)
    {
        for (Index_t val = 1; val <= 9; ++val) {
            if (cell.IsCandidate(val)) {
                cell.SetValue(val);
                return true;
            }
        }
        assert(false); // should not have a cell with 1 candidate and no true value
    }
    return false;
}

}
