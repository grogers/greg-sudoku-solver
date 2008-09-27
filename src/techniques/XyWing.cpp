#include "Sudoku.hpp"
#include "Logging.hpp"
#include <cassert>
#include <sstream>

namespace {
bool XyWingForCells(Sudoku &, Index_t, Index_t, Index_t, Index_t, Index_t, Index_t);

}

bool XyWing(Sudoku &sudoku)
{
    Log(Trace, "searching for xy-wings\n");

    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            if (sudoku.GetCell(i, j).NumCandidates() != 2)
                continue;

            boost::array<std::pair<Index_t, Index_t>, NUM_BUDDIES> buddies =
                sudoku.GetBuddies(i, j);

            boost::array<std::pair<Index_t, Index_t>, NUM_BUDDIES>::const_iterator it0, it1;
            for (it0 = buddies.begin(); it0 != buddies.end(); ++it0) {
                if (sudoku.GetCell(it0->first, it0->second).NumCandidates() != 2)
                    continue;
                for (it1 = it0 + 1; it1 < buddies.end(); ++it1) {
                    if (XyWingForCells(sudoku, i, j,
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
bool XyWingForCells(Sudoku &sudoku, Index_t xyRow, Index_t xyCol,
        Index_t xzRow, Index_t xzCol, Index_t yzRow, Index_t yzCol)
{
    // xy wing is all bivalue cells
    if (sudoku.GetCell(xyRow, xyCol).NumCandidates() != 2 ||
            sudoku.GetCell(xzRow, xzCol).NumCandidates() != 2 ||
            sudoku.GetCell(yzRow, yzCol).NumCandidates() != 2) {
        return false;
    }

    assert(xyRow != xzRow || xyCol != xzCol);
    assert(xyRow != yzRow || xyCol != yzCol);
    assert(xzRow != yzRow || xzCol != yzCol);

    assert(IsBuddy(xyRow, xyCol, xzRow, xzCol));
    assert(IsBuddy(xyRow, xyCol, yzRow, yzCol));

    Index_t x = 0, y = 0, z = 0;

    // find x from xy and xz
    for (Index_t val = 1; val <= 9; ++val) {
        if (sudoku.GetCell(xyRow, xyCol).IsCandidate(val) &&
                sudoku.GetCell(xzRow, xzCol).IsCandidate(val)) {
            x = val;
        }
    }

    if (x == 0)
        return false;

    // find y from xy - must succeed
    for (Index_t val = 1; val <= 9; ++val) {
        if (val != x && sudoku.GetCell(xyRow, xyCol).IsCandidate(val)) {
            y = val;
            break;
        }
    }

    // find z from xz - must succeed
    for (Index_t val = 1; val <= 9; ++val) {
        if (val != x && sudoku.GetCell(xzRow, xzCol).IsCandidate(val)) {
            z = val;
            break;
        }
    }

    if (x == y || x == z || y == z)
        return false;

    if (!sudoku.GetCell(yzRow, yzCol).IsCandidate(y) ||
            !sudoku.GetCell(yzRow, yzCol).IsCandidate(z))
        return false;

    // look for eliminations based on the found cells xz and yz and the value z
    boost::array<std::pair<Index_t, Index_t>, NUM_BUDDIES> xzBuddies =
        sudoku.GetBuddies(xzRow, xzCol);

    bool ret = false;
    std::vector<Index_t> changed;
    for (Index_t i = 0; i < NUM_BUDDIES; ++i) {
        if (IsBuddy(xzBuddies[i].first, xzBuddies[i].second, yzRow, yzCol) &&
                (xzBuddies[i].first != yzRow || xzBuddies[i].second != yzCol)) {
            Cell cell = sudoku.GetCell(xzBuddies[i].first, xzBuddies[i].second);
            if (cell.ExcludeCandidate(z)) {
                sudoku.SetCell(cell, xzBuddies[i].first, xzBuddies[i].second);
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
            changedStr << 'r' << xzBuddies[changed[i]].first+1 << 'c'
                << xzBuddies[changed[i]].second+1 << '#' << z;
        }

        Log(Info, "xy-wing (%d=%d)r%dc%d-(%d=%d)r%dc%d-(%d=%d)r%dc%d ==> %s\n",
                z, x, xzRow+1, xzCol+1,
                x, y, xyRow+1, xyCol+1,
                y, z, yzRow+1, yzCol+1,
                changedStr.str().c_str());
    }

    return ret;
}

}
