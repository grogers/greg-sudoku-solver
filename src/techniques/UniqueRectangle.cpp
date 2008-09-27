#include "Sudoku.hpp"
#include "Logging.hpp"

#include <algorithm>
#include <sstream>
#include <iostream>
#include <boost/tuple/tuple.hpp>

namespace {

boost::array<Cell, 4> GetCellsAtCornersOfUR(const Sudoku &, Index_t, Index_t,
        Index_t, Index_t);
bool UniqueRectangleType1(Sudoku &sudoku, Index_t, Index_t, Index_t, Index_t);
bool UniqueRectangleType25(Sudoku &sudoku, Index_t, Index_t, Index_t, Index_t);
void LogUniqueRectangle(Index_t, Index_t, Index_t, Index_t, Index_t, Index_t,
        Index_t, const std::vector<boost::tuple<Index_t, Index_t, Index_t> > &);
bool GetValuesOfUniqueRectangleType25(boost::array<Index_t, 2> &, Index_t &,
        const boost::array<Cell, 4> &, Index_t, Index_t);
bool EliminateValueFromBuddiesOfCells(Sudoku &, Index_t, Index_t, Index_t,
        Index_t, Index_t,
        std::vector<boost::tuple<Index_t, Index_t, Index_t> > &);
}

bool UniqueRectangle(Sudoku &sudoku)
{
    if (!sudoku.IsUnique()) {
        Log(Warning, "puzzle is not unique, unique rectangles may not be applied here\n");
        return false;
    }
    Log(Trace, "searching for unique rectangles\n");

    for (Index_t i0 = 0; i0 < 9; ++i0) {
        for (Index_t i1 = i0 + 1; i1 < 9; ++i1) {
            Index_t numBoxes;
            if (i0/3 == i1/3)
                numBoxes = 1;
            else
                numBoxes = 2;

            for (Index_t j0 = 0; j0 < 9; ++j0) {
                for (Index_t j1 = j0 + 1; j1 < 9; ++j1) {
                    if (numBoxes == 2 && j0/3 != j1/3)
                        continue;

                    if (UniqueRectangleType1(sudoku, i0, j0, i1, j1)) {
                        return true;
                    } else if (UniqueRectangleType25(sudoku, i0, j0, i1, j1)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


namespace {
bool UniqueRectangleType1(Sudoku &sudoku, Index_t row1, Index_t col1, Index_t row2, Index_t col2)
{
    boost::array<Cell, 4> corners =
        GetCellsAtCornersOfUR(sudoku, row1, col1, row2, col2);

    Index_t numCellsWith2Candidates = 0;
    Index_t cellWithout2Candidates = 0;
    for (Index_t i = 0; i < 4; ++i) {
        if (corners[i].NumCandidates() == 2)
            ++numCellsWith2Candidates;
        else
            cellWithout2Candidates = i;
    }
    if (numCellsWith2Candidates != 3)
        return false;

    Index_t numCandidates = 0;
    boost::array<Index_t, 2> values;
    for (Index_t i = 0; i < 4; ++i) {
        if (i == cellWithout2Candidates)
            continue;

        for (Index_t val = 1; val <= 9; ++val) {
            if (corners[i].IsCandidate(val) &&
                    std::find(values.data(), values.data() + numCandidates, val) ==
                    values.data() + numCandidates) {
                if (numCandidates >= 2)
                    return false;

                values[numCandidates++] = val;
            }
        }
    }
    if (numCandidates != 2)
        return false;

    std::vector<boost::tuple<Index_t, Index_t, Index_t> > changed;
    bool ret = false;

    for (Index_t i = 0; i < 2; ++i) {
        if (corners[cellWithout2Candidates].ExcludeCandidate(values[i])) {
            Index_t row, col;
            if (cellWithout2Candidates/2 == 0)
                row = row1;
            else
                row = row2;
            if (cellWithout2Candidates%2 == 0)
                col = col1;
            else
                col = col2;
            sudoku.SetCell(corners[cellWithout2Candidates], row, col);
            changed.push_back(boost::make_tuple(row, col, values[i]));
            ret = true;
        }
    }

    if (ret)
        LogUniqueRectangle(1, row1, col1, row2, col2, values[0], values[1],
                changed);

    return ret;
}

bool UniqueRectangleType25(Sudoku &sudoku, Index_t row1, Index_t col1, Index_t row2, Index_t col2)
{
    boost::array<Cell, 4> corners =
        GetCellsAtCornersOfUR(sudoku, row1, col1, row2, col2);

    Index_t numCellsWith2Candidates = 0, numCellsWith3Candidates = 0;
    boost::array<Index_t, 2> cellsWith3Candidates;
    for (Index_t i = 0; i < 4; ++i) {
        if (corners[i].NumCandidates() == 2) {
            ++numCellsWith2Candidates;
        } else if (corners[i].NumCandidates() == 3) {
            if (numCellsWith3Candidates >= 2)
                return false;
            cellsWith3Candidates[numCellsWith3Candidates++] = i;
        } else {
            return false;
        }
    }
    if (numCellsWith2Candidates != 2 || numCellsWith3Candidates != 2)
        return false;

    boost::array<Index_t, 2> values;
    Index_t extraValue;
    if (!GetValuesOfUniqueRectangleType25(values, extraValue, corners,
                cellsWith3Candidates[0], cellsWith3Candidates[1]))
        return false;

    std::vector<boost::tuple<Index_t, Index_t, Index_t> > changed;
    bool ret = false;

    boost::array<std::pair<Index_t, Index_t>, 2> cornerWith3Cells;

    for (Index_t i = 0; i < 2; ++i) {
        switch (cellsWith3Candidates[i]) {
            case 0:
                cornerWith3Cells[i].first = row1;
                cornerWith3Cells[i].second = col1;
                break;
            case 1:
                cornerWith3Cells[i].first = row1;
                cornerWith3Cells[i].second = col2;
                break;
            case 2:
                cornerWith3Cells[i].first = row2;
                cornerWith3Cells[i].second = col1;
                break;
            case 3:
                cornerWith3Cells[i].first = row2;
                cornerWith3Cells[i].second = col2;
                break;
            default:
                assert(false);
        }
    }

    if (EliminateValueFromBuddiesOfCells(sudoku,
                cornerWith3Cells[0].first,
                cornerWith3Cells[0].second,
                cornerWith3Cells[1].first,
                cornerWith3Cells[1].second, extraValue, changed)) {
        Index_t type = 2;
        if (cornerWith3Cells[0].first != cornerWith3Cells[1].first &&
                cornerWith3Cells[0].second != cornerWith3Cells[1].second)
            type = 5;

        LogUniqueRectangle(type, row1, col1, row2, col2, values[0], values[1],
                changed);
        ret = true;
    }
    return ret;
}


boost::array<Cell, 4> GetCellsAtCornersOfUR(const Sudoku &sudoku,
        Index_t row1, Index_t col1, Index_t row2, Index_t col2)
{
    boost::array<Cell, 4> ret;
    ret[0] = sudoku.GetCell(row1, col1);
    ret[1] = sudoku.GetCell(row1, col2);
    ret[2] = sudoku.GetCell(row2, col1);
    ret[3] = sudoku.GetCell(row2, col2);
    return ret;
}

void LogUniqueRectangle(Index_t type, Index_t row1, Index_t col1,
        Index_t row2, Index_t col2, Index_t val1, Index_t val2,
        const std::vector<boost::tuple<Index_t, Index_t, Index_t> > &changed)
{
    std::ostringstream changedStr;

    for (Index_t i = 0; i < changed.size(); ++i) {
        if (i != 0)
            changedStr << ", ";
        changedStr << 'r' << changed[i].get<0>()+1 << 'c'
            << changed[i].get<1>()+1 << '#' << changed[i].get<2>();
    }
    Log(Info, "type-%d unique rectangle r%d%dc%d%d=%d%d ==> %s\n",
            type, row1+1, row2+1, col1+1, col2+1, val1, val2,
            changedStr.str().c_str());
}

/**
 * Finds the values enclosed by the unique rectangle where the corners given
 * are the corners with extra candidates.
 * @param[out] values the 2 values that would form the deadly pattern
 * @param[out] extraVal the extra candidate in the UR
 * @return false if not a unique rectangle
 */
bool GetValuesOfUniqueRectangleType25(boost::array<Index_t, 2> &values,
        Index_t &extraVal, const boost::array<Cell, 4> &corners,
        Index_t corner1, Index_t corner2)
{
    // find the first corner not with extra candidates and get those values
    for (Index_t i = 0; i < 4; ++i) {
        if (i != corner1 && i != corner2) {
            Index_t cnt = 0;
            for (Index_t val = 1; val <= 9; ++val) {
                if (corners[i].IsCandidate(val)) {
                    assert(cnt < 2);
                    values[cnt++] = val;
                }
            }
            break;
        }
    }

    // make sure those candidates are present in all corners
    for (Index_t i = 0; i < 4; ++i) {
        if (!corners[i].IsCandidate(values[0]) ||
                !corners[i].IsCandidate(values[1]))
            return false;
    }

    // find the extra value
    for (Index_t val = 1; val <= 9; ++val) {
        if (val != values[0] && val != values[1] &&
                corners[corner1].IsCandidate(val)) {
            extraVal = val;
            break;
        }
    }

    return corners[corner2].IsCandidate(extraVal);
}

bool EliminateValueFromBuddiesOfCells(Sudoku &sudoku, Index_t row1,
        Index_t col1, Index_t row2, Index_t col2, Index_t value,
        std::vector<boost::tuple<Index_t, Index_t, Index_t> > &changed)
{
    bool ret = false;
    boost::array<std::pair<Index_t, Index_t>, NUM_BUDDIES> buddies =
        sudoku.GetBuddies(row1, col1);

    for (Index_t i = 0; i < NUM_BUDDIES; ++i) {
        Index_t row = buddies[i].first, col = buddies[i].second;
        if ((row == row1 && col == col1) || (row == row2 && col == col2))
            continue;

        if (IsBuddy(row, col, row2, col2)) {
            Cell cell = sudoku.GetCell(row, col);
            if (cell.ExcludeCandidate(value)) {
                sudoku.SetCell(cell, row, col);
                changed.push_back(boost::make_tuple(row, col, value));
                ret = true;
            }
        }
    }
    return ret;
}


}
