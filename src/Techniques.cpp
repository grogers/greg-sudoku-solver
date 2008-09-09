#include "Sudoku.hpp"
#include "Logging.hpp"

namespace {
    bool SelectBifurcationCell(const Sudoku &, Index_t &row, Index_t &col);
    bool NakedSingleInCell(Cell &);
    bool HiddenSingleInHouse(House &, Index_t &position, Index_t &val);
}

/**
 * Guess and Check.
 */
unsigned Bifurcate(Sudoku &sudoku, const std::vector<Technique> &techniques)
{
    StarryLog(Warning, 3, "Bifurcating... The output will now be for multiple puzzles, not just the one...\n");

    Index_t row, col, num;

    // if a cell cannot be found for bifurcation assume it cannot be solved
    if (!SelectBifurcationCell(sudoku, row, col))
        return 0;

    num = sudoku.GetCell(row, col).NumCandidates();
    Log(Debug, "Bifurcating on cell (%d,%d) with %d candidates\n", row, col, num);

    std::vector<Sudoku> newSudokus(num, sudoku);
    unsigned numSolved = 0;
    const Sudoku *solved = NULL;

    for (Index_t i = 1, idx = 0; i <= 9; ++i) {
        if (!sudoku.GetCell(row, col).IsCandidate(i))
            continue;

        Log(Trace, "Trying bifurcation on cell (%d,%d) of candidate %d\n", row, col, i);

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

    if (numSolved == 1) {
        Log(Trace, "Found a (potentially) unique solution\n");
        sudoku = *solved;
    }

    return numSolved;
}

/**
 * Only 1 possible candidate in a cell.
 */
bool NakedSingle(Sudoku &sudoku)
{
    Log(Trace, "Looking for Naked Singles\n");
    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            Cell cell = sudoku.GetCell(i, j);
            if (NakedSingleInCell(cell)) {
                Log(Info, "Found Naked Single in cell (%d,%d) with value %d\n", i, j, cell.GetValue());
                sudoku.SetCell(cell, i, j);
                sudoku.CrossHatch(i, j);
                return true;
            }
        }
    }
    return false;
}

/**
 * A given value has only one possible location in a house.
 */
bool HiddenSingle(Sudoku &sudoku)
{
    Log(Trace, "Looking for Hidden Singles\n");
    Index_t pos, val; // used only for logging purposes
    for (Index_t i = 0; i < 9; ++i) {
        House house = sudoku.GetRow(i);
        if (HiddenSingleInHouse(house, pos, val)) {
            Log(Info, "Found Hidden Single in cell (%d,%d) with value %d\n",
                    i, pos, val);
            Cell cell = sudoku.GetCell(i, pos);
            cell.SetValue(val);
            sudoku.SetCell(cell, i, pos);
            sudoku.CrossHatch(i, pos);
            return true;
        }

        house = sudoku.GetCol(i);
        if (HiddenSingleInHouse(house, pos, val)) {
            Log(Info, "Found Hidden Single in cell (%d,%d) with value %d\n",
                    pos, i, val);
            Cell cell = sudoku.GetCell(pos, i);
            cell.SetValue(val);
            sudoku.SetCell(cell, pos, i);
            sudoku.CrossHatch(pos, i);
            return true;
        }

        house = sudoku.GetBox(i);
        if (HiddenSingleInHouse(house, pos, val)) {
            Index_t row = RowForCellInBox(i, pos), col = ColForCellInBox(i, pos);
            Log(Info, "Found Hidden Single in cell (%d,%d) with value %d\n",
                    row, col, val);
            Cell cell = sudoku.GetCell(row, col);
            cell.SetValue(val);
            sudoku.SetCell(cell, row, col);
            sudoku.CrossHatch(row, col);
            return true;
        }

    }
    return false;
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

/**
 * @return true if a change was found.
 */
bool HiddenSingleInHouse(House &house, Index_t &position, Index_t &value)
{
    for (Index_t val = 1; val <= 9; ++val) {
        Index_t cnt = 0, pos = 0;

        for (Index_t i = 0; i < 9; ++i) {
            if (house[i].IsCandidate(val)) {
                ++cnt;
                pos = i;
            }
        }

        if (cnt == 1) {
            // changing the cell will be done by the calling function
            position = pos;
            value = val;
            return true;
        }
    }
    return false;
}


}
