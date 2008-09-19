#include "Sudoku.hpp"
#include "Logging.hpp"
#include "LockedSet.hpp"

#include <sstream>

namespace {
bool BasicFishForValue(Sudoku &, Index_t val);
std::vector<Index_t> IndicesOfPossibleRowBaseBasicFish(const Sudoku &, Index_t);
bool HouseHasValueSet(const House &, Index_t);
std::vector<Index_t> IndicesOfPossibleColBaseBasicFish(const Sudoku &, Index_t);
bool RowBaseBasicFish(Sudoku &, Index_t val);
bool RowBaseBasicFishWithIndices(Sudoku &, Index_t val, boost::array<Index_t, 4> &indices, Index_t order);
bool ColBaseBasicFish(Sudoku &, Index_t val);
bool ColBaseBasicFishWithIndices(Sudoku &, Index_t val, boost::array<Index_t, 4> &indices, Index_t order);
struct ChangedCell
{
    ChangedCell(Index_t _row, Index_t _col, Index_t _val) : row(_row), col(_col), val(_val) {}
    Index_t row;
    Index_t col;
    Index_t val;
};
const char *OrderToString(Index_t order);
}

bool BasicFish(Sudoku &sudoku)
{
    Log(Trace, "searching for basic fish\n");
    for (Index_t val = 1; val <= 9; ++val) {
        if (BasicFishForValue(sudoku, val))
            return true;
    }
    return false;
}


namespace {
bool BasicFishForValue(Sudoku &sudoku, Index_t val)
{
    if (RowBaseBasicFish(sudoku, val))
        return true;
    if (ColBaseBasicFish(sudoku, val))
        return true;

    return false;
}

std::vector<Index_t> IndicesOfPossibleRowBaseBasicFish(const Sudoku &sudoku, Index_t val)
{
    std::vector<Index_t> ret;
    for (Index_t i = 0; i < 9; ++i) {
        if (!HouseHasValueSet(sudoku.GetRow(i), val))
            ret.push_back(i);
    }
    return ret;
}

bool HouseHasValueSet(const House &house, Index_t val)
{
    for (Index_t i = 0; i < 9; ++i) {
        if (house[i].HasValue() && house[i].GetValue() == val)
            return true;
    }
    return false;
}

std::vector<Index_t> IndicesOfPossibleColBaseBasicFish(const Sudoku &sudoku, Index_t val)
{
    std::vector<Index_t> ret;
    for (Index_t i = 0; i < 9; ++i) {
        if (!HouseHasValueSet(sudoku.GetCol(i), val))
            ret.push_back(i);
    }
    return ret;
}

bool RowBaseBasicFish(Sudoku &sudoku, Index_t val)
{
    std::vector<Index_t> indexList = IndicesOfPossibleRowBaseBasicFish(sudoku, val);
    for (Index_t order = 2; order <= indexList.size()/2; ++order) {
        std::vector<Index_t> indicesToVisit(order);
        for (Index_t i = 0; i < order; ++i)
            indicesToVisit[i] = i;

        do {
            boost::array<Index_t, 4> rowIndices;
            for (Index_t i = 0; i < order; ++i)
                rowIndices[i] = indexList[indicesToVisit[i]];

            if (RowBaseBasicFishWithIndices(sudoku, val, rowIndices, order))
                return true;
        } while (GetNewIndicesToVisit(indicesToVisit, indexList.size()));
    }
    return false;
}

bool RowBaseBasicFishWithIndices(Sudoku &sudoku, Index_t val, boost::array<Index_t, 4> &rowIndices, Index_t order)
{
    bool ret = false;
    boost::array<Index_t, 4> colIndices = {{ 0 }};
    Index_t numCols = 0;

    for (Index_t i = 0; i < order; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            if (sudoku.GetCell(rowIndices[i], j).IsCandidate(val) &&
                    std::find(colIndices.data(), colIndices.data() + numCols, j) == colIndices.data() + numCols) {
                if (numCols >= order)
                    return false;

                colIndices[numCols++] = j;
            }
        }
    }

    if (numCols == order) {
        std::vector<ChangedCell> changed;
        for (Index_t i = 0; i < 9; ++i) {
            if (std::find(rowIndices.data(), rowIndices.data() + order, i) != rowIndices.data() + order)
                continue;

            for (Index_t j = 0; j < order; ++j) {
                Cell cell = sudoku.GetCell(i, colIndices[j]);
                if (cell.ExcludeCandidate(val))
                {
                    changed.push_back(ChangedCell(i, colIndices[j], val));
                    sudoku.SetCell(cell, i, colIndices[j]);
                    ret = true;
                }
            }
        }

        if (ret) {
            std::ostringstream fishStr, changedStr;
            fishStr << 'r';
            for (Index_t i = 0; i < order; ++i)
                fishStr << rowIndices[i]+1;
            fishStr << "\\c";
            for (Index_t i = 0; i < order; ++i)
                fishStr << colIndices[i]+1;
            fishStr << '=' << val;

            for (std::vector<ChangedCell>::const_iterator i = changed.begin();
                    i != changed.end(); ++i) {
                if (i != changed.begin())
                    changedStr << ", ";
                changedStr << 'r' << i->row+1 << 'c' << i->col+1 << '#' << i->val;
            }

            Log(Info, "basic %s %s ==> %s\n", OrderToString(order),
                    fishStr.str().c_str(), changedStr.str().c_str());
        }
    }
    return ret;
}

bool ColBaseBasicFish(Sudoku &sudoku, Index_t val)
{
    std::vector<Index_t> indexList = IndicesOfPossibleColBaseBasicFish(sudoku, val);
    for (Index_t order = 2; order <= indexList.size()/2; ++order) {
        std::vector<Index_t> indicesToVisit(order);
        for (Index_t i = 0; i < order; ++i)
            indicesToVisit[i] = i;

        do {
            boost::array<Index_t, 4> colIndices;
            for (Index_t i = 0; i < order; ++i)
                colIndices[i] = indexList[indicesToVisit[i]];

            if (ColBaseBasicFishWithIndices(sudoku, val, colIndices, order))
                return true;
        } while (GetNewIndicesToVisit(indicesToVisit, indexList.size()));
    }
    return false;
}

bool ColBaseBasicFishWithIndices(Sudoku &sudoku, Index_t val, boost::array<Index_t, 4> &colIndices, Index_t order)
{
    bool ret = false;
    boost::array<Index_t, 4> rowIndices = {{ 0 }};
    Index_t numRows = 0;

    for (Index_t i = 0; i < order; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            if (sudoku.GetCell(j, colIndices[i]).IsCandidate(val) &&
                    std::find(rowIndices.data(), rowIndices.data() + numRows, j) == rowIndices.data() + numRows) {
                if (numRows >= order)
                    return false;

                rowIndices[numRows++] = j;
            }
        }
    }

    if (numRows == order) {
        std::vector<ChangedCell> changed;
        for (Index_t i = 0; i < 9; ++i) {
            if (std::find(colIndices.data(), colIndices.data() + order, i) != colIndices.data() + order)
                continue;

            for (Index_t j = 0; j < order; ++j) {
                Cell cell = sudoku.GetCell(rowIndices[j], i);
                if (cell.ExcludeCandidate(val))
                {
                    changed.push_back(ChangedCell(rowIndices[j], i, val));
                    sudoku.SetCell(cell, rowIndices[j], i);
                    ret = true;
                }
            }
        }

        if (ret) {
            std::ostringstream fishStr, changedStr;
            fishStr << 'c';
            for (Index_t i = 0; i < order; ++i)
                fishStr << colIndices[i]+1;
            fishStr << "/r";
            for (Index_t i = 0; i < order; ++i)
                fishStr << rowIndices[i]+1;
            fishStr << '=' << val;

            for (std::vector<ChangedCell>::const_iterator i = changed.begin();
                    i != changed.end(); ++i) {
                if (i != changed.begin())
                    changedStr << ", ";
                changedStr << 'r' << i->row+1 << 'c' << i->col+1 << '#' << i->val;
            }

            Log(Info, "basic %s %s ==> %s\n", OrderToString(order),
                    fishStr.str().c_str(), changedStr.str().c_str());
        }
    }
    return ret;
}

const char *OrderToString(Index_t order)
{
    switch (order) {
        case 1: return "1-fish";
        case 2: return "x-wing";
        case 3: return "swordfish";
        case 4: return "jellyfish";
        case 5: return "squirmbag";
        case 6: return "whale";
        case 7: return "leviathan";
        default: return "unknown";
    }
}

}
