#include "Sudoku.hpp"
#include "Logging.hpp"
#include <cassert>
#include <sstream>

namespace {
bool XyzWingForCells(Sudoku &, Index_t, Index_t, Index_t, Index_t, Index_t, Index_t);

}

bool XyzWing(Sudoku &sudoku)
{
    Log(Trace, "searching for xyz-wings\n");

    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            if (sudoku.GetCell(i, j).NumCandidates() != 3)
                continue;

            boost::array<std::pair<Index_t, Index_t>, NUM_BUDDIES> buddies =
                sudoku.GetBuddies(i, j);

            boost::array<std::pair<Index_t, Index_t>, NUM_BUDDIES>::const_iterator it0, it1;
            for (it0 = buddies.begin(); it0 != buddies.end(); ++it0) {
                if (sudoku.GetCell(it0->first, it0->second).NumCandidates() != 2)
                    continue;
                for (it1 = it0 + 1; it1 < buddies.end(); ++it1) {
                    if (XyzWingForCells(sudoku, i, j,
                                it0->first, it0->second, it1->first,
                                it1->second)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}



namespace {
bool XyzWingForCells(Sudoku &sudoku, Index_t xyzRow, Index_t xyzCol,
        Index_t xzRow, Index_t xzCol, Index_t yzRow, Index_t yzCol)
{
    // xy wing is all bivalue cells
    if (sudoku.GetCell(xyzRow, xyzCol).NumCandidates() != 3 ||
            sudoku.GetCell(xzRow, xzCol).NumCandidates() != 2 ||
            sudoku.GetCell(yzRow, yzCol).NumCandidates() != 2) {
        return false;
    }

    assert(xyzRow != xzRow || xyzCol != xzCol);
    assert(xyzRow != yzRow || xyzCol != yzCol);
    assert(xzRow != yzRow || xzCol != yzCol);

    assert(IsBuddy(xyzRow, xyzCol, xzRow, xzCol));
    assert(IsBuddy(xyzRow, xyzCol, yzRow, yzCol));

    Index_t x = 0, y = 0, z = 0;

    // find z from all the cells
    for (Index_t val = 1; val <= 9; ++val) {
        if (sudoku.GetCell(xyzRow, xyzCol).IsCandidate(val) &&
                sudoku.GetCell(xzRow, xzCol).IsCandidate(val) &&
                sudoku.GetCell(yzRow, yzCol).IsCandidate(val)) {
            z = val;
        }
    }

    if (z == 0)
        return false;

    // find x from xz - must succeed
    for (Index_t val = 1; val <= 9; ++val) {
        if (val != z && sudoku.GetCell(xzRow, xzCol).IsCandidate(val)) {
            x = val;
            break;
        }
    }

    // find y from yz - must succeed
    for (Index_t val = 1; val <= 9; ++val) {
        if (val != z && sudoku.GetCell(yzRow, yzCol).IsCandidate(val)) {
            y = val;
            break;
        }
    }

    if (x == y || x == z || y == z)
        return false;

    if (!sudoku.GetCell(xyzRow, xyzCol).IsCandidate(x) ||
            !sudoku.GetCell(xyzRow, xyzCol).IsCandidate(y))
        return false;

    // eliminations must be buddies with all the cells
    boost::array<std::pair<Index_t, Index_t>, NUM_BUDDIES> xyzBuddies =
        sudoku.GetBuddies(xyzRow, xyzCol);

    bool ret = false;
    std::vector<Index_t> changed;
    for (Index_t i = 0; i < NUM_BUDDIES; ++i) {
        if (IsBuddy(xyzBuddies[i].first, xyzBuddies[i].second, xzRow, xzCol) &&
                IsBuddy(xyzBuddies[i].first, xyzBuddies[i].second, yzRow, yzCol) &&
                (xyzBuddies[i].first != xyzRow || xyzBuddies[i].second != xyzCol) &&
                (xyzBuddies[i].first != xzRow || xyzBuddies[i].second != xzCol) &&
                (xyzBuddies[i].first != yzRow || xyzBuddies[i].second != yzCol)) {
            Cell cell = sudoku.GetCell(xyzBuddies[i].first, xyzBuddies[i].second);
            if (cell.ExcludeCandidate(z)) {
                sudoku.SetCell(cell, xyzBuddies[i].first, xyzBuddies[i].second);
                changed.push_back(i);
                ret = true;
            }
        }
    }

    if (ret) {
        std::ostringstream changedStr;
        for (Index_t i = 0; i < changed.size(); ++i) {
            if (i != 0)
                changedStr << ", ";
            changedStr << 'r' << xyzBuddies[changed[i]].first+1 << 'c'
                << xyzBuddies[changed[i]].second+1 << '#' << z;
        }

        Log(Info, "xyz-wing r%dc%d=%d%d, r%dc%d=%d%d%d, r%dc%d=%d%d ==> %s\n",
                xzRow+1, xzCol+1, x, z,
                xyzRow+1, xyzCol+1, x, y, z,
                yzRow+1, yzCol+1, y, z,
                changedStr.str().c_str());
    }

    return ret;
}

}
