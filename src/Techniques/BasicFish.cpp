#include "Sudoku.hpp"
#include "Logging.hpp"
#include "LockedSet.hpp"

#include <sstream>

#include <boost/tuple/tuple.hpp>

namespace {
typedef boost::tuple<Index_t, Index_t, Index_t> RowColVal;

Index_t MaxSizeOfBasicFish(const Sudoku &, Index_t value);
std::vector<Index_t> IndicesOfPossibleRowBaseBasicFish(const Sudoku &, Index_t, Index_t);
std::vector<Index_t> IndicesOfPossibleColBaseBasicFish(const Sudoku &, Index_t, Index_t);
bool RowBaseBasicFish(Sudoku &, Index_t val);
bool RowBaseBasicFishWithIndices(Sudoku &, Index_t val, boost::array<Index_t, 4> &indices, Index_t order);
bool ColBaseBasicFish(Sudoku &, Index_t val);
bool ColBaseBasicFishWithIndices(Sudoku &, Index_t val, boost::array<Index_t, 4> &indices, Index_t order);
void LogBasicFish(bool rowBase, const boost::array<Index_t, 4> &rows,
        const boost::array<Index_t, 4> &cols,
        const std::vector<RowColVal> &changed,
        Index_t value, Index_t order);
const char *OrderToString(Index_t order);
}

bool BasicFish(Sudoku &sudoku)
{
    Log(Trace, "searching for basic fish\n");
    for (Index_t val = 1; val <= 9; ++val) {
        if (RowBaseBasicFish(sudoku, val))
            return true;
        if (ColBaseBasicFish(sudoku, val))
            return true;
    }
    return false;
}


namespace {
Index_t MaxSizeOfBasicFish(const Sudoku &sudoku, Index_t value)
{
    Index_t cnt = 0;
    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            Cell cell = sudoku.GetCell(i, j);
            if (cell.HasValue() && cell.GetValue() == value)
                ++cnt;
        }
    }
    return (9 - cnt)/2;
}


std::vector<Index_t> IndicesOfPossibleRowBaseBasicFish(const Sudoku &sudoku, Index_t value, Index_t order)
{
    std::vector<Index_t> ret;
    for (Index_t i = 0; i < 9; ++i) {
        Index_t num = NumTimesValueOpenInHouse(sudoku.GetRow(i), value);
        if (num <= order && num != 0)
            ret.push_back(i);
    }
    return ret;
}

std::vector<Index_t> IndicesOfPossibleColBaseBasicFish(const Sudoku &sudoku, Index_t value, Index_t order)
{
    std::vector<Index_t> ret;
    for (Index_t i = 0; i < 9; ++i) {
        Index_t num = NumTimesValueOpenInHouse(sudoku.GetCol(i), value);
        if (num <= order && num != 0)
            ret.push_back(i);
    }
    return ret;
}

bool RowBaseBasicFish(Sudoku &sudoku, Index_t value)
{
    Index_t max = MaxSizeOfBasicFish(sudoku, value);
    for (Index_t order = 2; order <= max; ++order) {
        std::vector<Index_t> indices =
            IndicesOfPossibleRowBaseBasicFish(sudoku, value, order);
        if (indices.size() < order)
            continue;

        std::vector<Index_t> indicesToVisit(order);
        for (Index_t i = 0; i < order; ++i)
            indicesToVisit[i] = i;

        do {
            boost::array<Index_t, 4> rowIndices;
            for (Index_t i = 0; i < order; ++i)
                rowIndices[i] = indices[indicesToVisit[i]];

            if (RowBaseBasicFishWithIndices(sudoku, value, rowIndices, order))
                return true;
        } while (GetNewIndicesToVisit(indicesToVisit, indices.size()));
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
        std::vector<RowColVal> changed;
        for (Index_t i = 0; i < 9; ++i) {
            if (std::find(rowIndices.data(), rowIndices.data() + order, i) != rowIndices.data() + order)
                continue;

            for (Index_t j = 0; j < order; ++j) {
                Cell cell = sudoku.GetCell(i, colIndices[j]);
                if (cell.ExcludeCandidate(val))
                {
                    changed.push_back(RowColVal(i, colIndices[j], val));
                    sudoku.SetCell(cell, i, colIndices[j]);
                    ret = true;
                }
            }
        }

        if (ret)
            LogBasicFish(true, rowIndices, colIndices, changed, val, order);
    }
    return ret;
}

bool ColBaseBasicFish(Sudoku &sudoku, Index_t value)
{
    Index_t max = MaxSizeOfBasicFish(sudoku, value);
    for (Index_t order = 2; order <= max; ++order) {
        std::vector<Index_t> indices =
            IndicesOfPossibleColBaseBasicFish(sudoku, value, order);
        if (indices.size() < order)
            continue;

        std::vector<Index_t> indicesToVisit(order);
        for (Index_t i = 0; i < order; ++i)
            indicesToVisit[i] = i;

        do {
            boost::array<Index_t, 4> colIndices;
            for (Index_t i = 0; i < order; ++i)
                colIndices[i] = indices[indicesToVisit[i]];

            if (ColBaseBasicFishWithIndices(sudoku, value, colIndices, order))
                return true;
        } while (GetNewIndicesToVisit(indicesToVisit, indices.size()));
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
        std::vector<RowColVal> changed;
        for (Index_t i = 0; i < 9; ++i) {
            if (std::find(colIndices.data(), colIndices.data() + order, i) != colIndices.data() + order)
                continue;

            for (Index_t j = 0; j < order; ++j) {
                Cell cell = sudoku.GetCell(rowIndices[j], i);
                if (cell.ExcludeCandidate(val))
                {
                    changed.push_back(RowColVal(rowIndices[j], i, val));
                    sudoku.SetCell(cell, rowIndices[j], i);
                    ret = true;
                }
            }
        }

        if (ret)
            LogBasicFish(false, rowIndices, colIndices, changed, val, order);
    }
    return ret;
}

void LogBasicFish(bool rowBase, const boost::array<Index_t, 4> &rows,
        const boost::array<Index_t, 4> &cols,
        const std::vector<RowColVal> &changed,
        Index_t value, Index_t order)
{
    std::ostringstream fishStr, changedStr;

    fishStr << (rowBase?'r':'c');
    for (Index_t i = 0; i < order; ++i)
        fishStr << (rowBase?rows[i]+1:cols[i]+1);
    fishStr << '/' << (rowBase?'c':'r');
    for (Index_t i = 0; i < order; ++i)
        fishStr << (rowBase?cols[i]+1:rows[i]+1);
    fishStr << '=' << value;

    for (std::vector<RowColVal>::const_iterator i = changed.begin();
            i != changed.end(); ++i) {
        if (i != changed.begin())
            changedStr << ", ";
        changedStr << 'r' << i->get<0>()+1 << 'c' << i->get<1>()+1 << '#' << i->get<2>();
    }

    Log(Info, "%s %s ==> %s\n", OrderToString(order),
            fishStr.str().c_str(), changedStr.str().c_str());
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
