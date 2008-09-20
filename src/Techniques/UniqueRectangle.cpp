#include "Sudoku.hpp"
#include "Logging.hpp"

#include <algorithm>
#include <sstream>
#include <iostream>

namespace {
bool UniqueRectangleType1(Sudoku &sudoku, Index_t row1, Index_t row2, Index_t col1, Index_t col2);
}

bool UniqueRectangle(Sudoku &sudoku)
{
    if (!sudoku.IsUnique()) {
        Log(Warning, "puzzle is not unique, unique rectangles may not be applied here\n");
        return false;
    }

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

                    if (UniqueRectangleType1(sudoku, i0, i1, j0, j1)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


namespace {
bool UniqueRectangleType1(Sudoku &sudoku, Index_t row1, Index_t row2, Index_t col1, Index_t col2)
{
    boost::array<Cell, 4> corners;
    corners[0] = sudoku.GetCell(row1, col1);
    corners[1] = sudoku.GetCell(row1, col2);
    corners[2] = sudoku.GetCell(row2, col1);
    corners[3] = sudoku.GetCell(row2, col2);

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

    std::vector<Index_t> changed;
    bool ret = false;

    for (Index_t i = 0; i < 2; ++i) {
        if (corners[cellWithout2Candidates].ExcludeCandidate(values[i])) {
            changed.push_back(i);
            ret = true;
        }
    }

    if (ret) {
        std::ostringstream changedStr;

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

        for (Index_t i = 0; i < changed.size(); ++i) {
            if (i != 0)
                changedStr << ", ";
            changedStr << 'r' << row+1 << 'c' << col2+1 << '#'
                << values[changed[i]];
        }
        Log(Info, "type-1 unique rectangle r%d%dc%d%d=%d%d ==> %s\n",
                row1+1, row2+1, col1+1, col2+1, values[0], values[1],
                changedStr.str().c_str());
    }

    return ret;
}





}
