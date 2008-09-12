#include "Sudoku.hpp"
#include "Logging.hpp"

namespace {
bool HiddenSingleInHouse(House &, Index_t &position, Index_t &val);
}

/**
 * A given value has only one possible location in a house.
 */
bool HiddenSingle(Sudoku &sudoku)
{
    Log(Trace, "searching for hidden singles\n");
    bool ret = false; // optimization - keep looking for more hidden singles instead of just 1
    Index_t pos, val; // used only for logging purposes
    for (Index_t i = 0; i < 9; ++i) {
        House house = sudoku.GetRow(i);
        if (HiddenSingleInHouse(house, pos, val)) {
            Log(Info, "hidden single in row ==> r%dc%d = %d\n",
                    i+1, pos+1, val);
            Cell cell = sudoku.GetCell(i, pos);
            cell.SetValue(val);
            sudoku.SetCell(cell, i, pos);
            sudoku.CrossHatch(i, pos);
            ret = true;
        }

        house = sudoku.GetCol(i);
        if (HiddenSingleInHouse(house, pos, val)) {
            Log(Info, "hidden single in column ==> r%dc%d = %d\n",
                    pos+1, i+1, val);
            Cell cell = sudoku.GetCell(pos, i);
            cell.SetValue(val);
            sudoku.SetCell(cell, pos, i);
            sudoku.CrossHatch(pos, i);
            ret = true;
        }

        house = sudoku.GetBox(i);
        if (HiddenSingleInHouse(house, pos, val)) {
            Index_t row = RowForCellInBox(i, pos);
            Index_t col = ColForCellInBox(i, pos);
            Log(Info, "hidden single in box ==> r%dc%d = %d\n",
                    row+1, col+1, val);
            Cell cell = sudoku.GetCell(row, col);
            cell.SetValue(val);
            sudoku.SetCell(cell, row, col);
            sudoku.CrossHatch(row, col);
            ret = true;
        }
    }
    return ret;
}

namespace {

/**
 * @return true if a change was found.
 * @note position and value are used to get the cell changed.
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
