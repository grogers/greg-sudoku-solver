#include "Sudoku.hpp"
#include "Logging.hpp"

#include <algorithm>

namespace {
    bool SelectBifurcationCell(const Sudoku &, Index_t &row, Index_t &col);
    bool NakedSingleInCell(Cell &);
    bool HiddenSingleInHouse(House &, Index_t &position, Index_t &val);

    // first index (0-1) is the input house reference, second index (0-2) is a
    // counter of cell number (3 cells are common between a box and a line)
    typedef boost::array<boost::array<Index_t, 3>, 2> CommonCells;
    CommonCells CommonCellsBoxRow(Index_t box, Index_t row);
    CommonCells CommonCellsBoxCol(Index_t box, Index_t col);
    CommonCells CommonCellsRowBox(Index_t row, Index_t box);
    CommonCells CommonCellsColBox(Index_t col, Index_t box);
    CommonCells ReverseCommonCells(const CommonCells &);
    bool IntersectionOfHouses(House &, const House &, const CommonCells &);
    bool AreAllCandidatesInHouseInCommonCells(const House &, Index_t val, const boost::array<Index_t, 3> &);
    bool RemoveAllCandidatesInHouseNotInCommonCells(House &, Index_t val, const boost::array<Index_t, 3> &);

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
    bool ret = false;

    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            Cell cell = sudoku.GetCell(i, j);
            if (NakedSingleInCell(cell)) {
                Log(Info, "Found Naked Single in cell (%d,%d) with value %d\n", i, j, cell.GetValue());
                sudoku.SetCell(cell, i, j);
                sudoku.CrossHatch(i, j);
                ret = true; // optimization - keep looping until all are found
            }
        }
    }
    return ret;
}

/**
 * A given value has only one possible location in a house.
 */
bool HiddenSingle(Sudoku &sudoku)
{
    Log(Trace, "Looking for Hidden Singles\n");
    bool ret = false; // optimization - keep looking for more hidden singles instead of just 1
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
            ret = true;
        }

        house = sudoku.GetCol(i);
        if (HiddenSingleInHouse(house, pos, val)) {
            Log(Info, "Found Hidden Single in cell (%d,%d) with value %d\n",
                    pos, i, val);
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
            Log(Info, "Found Hidden Single in cell (%d,%d) with value %d\n",
                    row, col, val);
            Cell cell = sudoku.GetCell(row, col);
            cell.SetValue(val);
            sudoku.SetCell(cell, row, col);
            sudoku.CrossHatch(row, col);
            ret = true;
        }
    }
    return ret;
}
#include <iostream>
bool IntersectionRemoval(Sudoku &sudoku)
{
    Log(Trace, "Looking for Box/Line Intersection Removal\n");
    for (Index_t i = 0; i < 9; ++i) {
        House line = sudoku.GetRow(i);
        for (Index_t j = 0; j < 3; ++j)
        {
            Index_t boxIndex = BoxIndex(i, j*3);
            House box = sudoku.GetBox(boxIndex);
            if (IntersectionOfHouses(line, box, CommonCellsRowBox(i, boxIndex))) {
                Log(Info, "Found an Intersection Removal at Row %d, Box %d\n", i, boxIndex);
                sudoku.SetRow(line, i);
                return true;
            }

            if (IntersectionOfHouses(box, line, CommonCellsBoxRow(boxIndex, i))) {
                Log(Info, "Found an Intersection Removal at Box %d, Row %d\n", boxIndex, i);
                sudoku.SetBox(box, boxIndex);
                return true;
            }
        }

        line = sudoku.GetCol(i);
        for (Index_t j = 0; j < 3; ++j)
        {
            Index_t boxIndex = BoxIndex(j*3, i);
            House box = sudoku.GetBox(boxIndex);
            if (IntersectionOfHouses(line, box, CommonCellsColBox(i, boxIndex))) {
                Log(Info, "Found an Intersection Removal at Col %d, Box %d\n", i, boxIndex);
                sudoku.SetCol(line, i);
                return true;
            }

            if (IntersectionOfHouses(box, line, CommonCellsBoxCol(boxIndex, i))) {
                Log(Info, "Found an Intersection Removal at Box %d, Col %d\n", boxIndex, i);
                sudoku.SetBox(box, boxIndex);
                return true;
            }
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

CommonCells CommonCellsBoxRow(Index_t box, Index_t row)
{
    assert(row/3 == box/3); // logic error if the box and the line don't intersect

    CommonCells cells;
    for (Index_t i = 0; i < 3; ++i) {
        cells[0][i] = (row%3)*3 + i;
        cells[1][i] = (box%3)*3 + i;
    }
    return cells;
}

CommonCells CommonCellsBoxCol(Index_t box, Index_t col)
{
    assert(col/3 == box%3); // logic error if the box and the line don't intersect

    CommonCells cells;
    for (Index_t i = 0; i < 3; ++i) {
        cells[0][i] = col%3 + i*3;
        cells[1][i] = (box/3)*3 + i;
    }
    return cells;
}

CommonCells CommonCellsRowBox(Index_t row, Index_t box)
{
    return ReverseCommonCells(CommonCellsBoxRow(box, row));
}

CommonCells CommonCellsColBox(Index_t col, Index_t box)
{
    return ReverseCommonCells(CommonCellsBoxCol(box, col));
}

CommonCells ReverseCommonCells(const CommonCells &cells)
{
    CommonCells ret = cells;
    ret[0].swap(ret[1]);
    return ret;
}

/**
 * If all the candidates of a given value in house2 occur in the cells common to
 * house1 and house2, then any cell in house1 not in these common cells cannot be
 * that value.
 */
bool IntersectionOfHouses(House &house1, const House &house2, const CommonCells &commonCells)
{
    for (Index_t val = 1; val <= 9; ++val) {
        if (!AreAllCandidatesInHouseInCommonCells(house2, val, commonCells[1]))
            continue;

        if (RemoveAllCandidatesInHouseNotInCommonCells(house1, val, commonCells[0]))
            return true;
    }
    return false;
}

bool AreAllCandidatesInHouseInCommonCells(const House &house, Index_t val,
        const boost::array<Index_t, 3> &cells)
{
    Index_t numFound = 0;
    for (Index_t i = 0; i < 9; ++i) {
        if (!house[i].IsCandidate(val))
            continue;

        if (std::find(cells.begin(), cells.end(), i) == cells.end())
            return false;

        ++numFound;
    }
    return numFound > 0;
}

bool RemoveAllCandidatesInHouseNotInCommonCells(House &house, Index_t val,
        const boost::array<Index_t, 3> &cells)
{
    Index_t numFound = 0;
    for (Index_t i = 0; i < 9; ++i) {
        if (std::find(cells.begin(), cells.end(), i) != cells.end())
            continue;

        if (house[i].ExcludeCandidate(val))
            ++numFound;
    }
    return numFound > 0;
}


}
