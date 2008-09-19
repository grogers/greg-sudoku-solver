#include "Sudoku.hpp"
#include "Logging.hpp"
#include "Options.hpp"
#include "Techniques.hpp"

#include <istream>
#include <ostream>
#include <cctype>
#include <algorithm>

namespace {
    void OutputByValue(const Sudoku &, std::ostream &);
    Index_t GetMaxNumCandidatesInColumn(const Sudoku &, Index_t);
    boost::array<Index_t, 9> GetMaxNumCandidatesAllColumns(const Sudoku &);
    void PrintLineSeparator(const boost::array<Index_t, 9> &, std::ostream &);
    void OutputByCandidates(const Sudoku &, std::ostream &);
    bool InputByValue(Sudoku &, std::istream &);
    bool InputByCandidates(Sudoku &, std::istream &);
    bool IsValueInHouse(const House &, Index_t);
    bool AreAllValuesInHouse(const House &);
    bool TryAllTechniques(Sudoku &, const std::vector<Technique> &);
    bool AllCellsHaveValues(const Sudoku &);
    bool AnyCellsBlank(const Sudoku &);
}

Sudoku &Sudoku::operator=(const Sudoku &x)
{
    _board = x._board;
    _unique = x._unique;
    return *this;
}

void Sudoku::Reset()
{
    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            _board[i][j] = Cell();
        }
    }
    _unique = boost::logic::indeterminate;
}

/**
 * After setting the value of a given cell, eliminate that candidate from cells
 * that can see that cell.
 */
void Sudoku::CrossHatch(Index_t row, Index_t col)
{
    Index_t value = _board[row][col].GetValue();

    House house = GetRow(row);
    for (Index_t i = 0; i < 9; ++i)
        house[i].ExcludeCandidate(value);
    SetRow(house, row);

    house = GetCol(col);
    for (Index_t i = 0; i < 9; ++i)
        house[i].ExcludeCandidate(value);
    SetCol(house, col);

    house = GetBox(BoxIndex(row, col));
    for (Index_t i = 0; i < 9; ++i)
        house[i].ExcludeCandidate(value);
    SetBox(house, BoxIndex(row, col));
}

House Sudoku::GetRow(Index_t row) const
{
    House house;
    for (Index_t i = 0; i < 9; ++i)
        house[i] = _board[row][i];
    return house;
}

void Sudoku::SetRow(const House &house, Index_t row)
{
    for (Index_t i = 0; i < 9; ++i)
        _board[row][i] = house[i];
}


House Sudoku::GetCol(Index_t col) const
{
    House house;
    for (Index_t i = 0; i < 9; ++i)
        house[i] = _board[i][col];
    return house;
}

void Sudoku::SetCol(const House &house, Index_t col)
{
    for (Index_t i = 0; i < 9; ++i)
        _board[i][col] = house[i];
}


House Sudoku::GetBox(Index_t box) const
{
    House house;
    Index_t n = 0;
    for (Index_t i = (box/3)*3; i < (box/3)*3 + 3; ++i) {
        for (Index_t j = (box%3)*3; j < (box%3)*3 + 3; ++j) {
            house[n] = _board[i][j];
            ++n;
        }
    }
    return house;
}

void Sudoku::SetBox(const House &house, Index_t box)
{
    Index_t n = 0;
    for (Index_t i = (box/3)*3; i < (box/3)*3 + 3; ++i) {
        for (Index_t j = (box%3)*3; j < (box%3)*3 + 3; ++j) {
            _board[i][j] = house[n];
            ++n;
        }
    }
}

boost::array<std::pair<Index_t, Index_t>, NUM_BUDDIES> Sudoku::GetBuddies(Index_t row, Index_t col) const
{
    boost::array<std::pair<Index_t, Index_t>, NUM_BUDDIES> ret;
    Index_t j = 0;

    for (Index_t i = 0; i < 9; ++i) {
        if (i != col &&
                std::find(ret.data(), ret.data() + j, std::make_pair(row, i)) ==  ret.data() + j) {
            ret[j++] = std::make_pair(row, i);
        }
    }

    for (Index_t i = 0; i < 9; ++i) {
        if (i != row &&
                std::find(ret.data(), ret.data() + j, std::make_pair(i, col)) ==  ret.data() + j) {
            ret[j++] = std::make_pair(i, col);
        }
    }

    Index_t boxIndex = BoxIndex(row, col);
    for (Index_t i = 0; i < 9; ++i) {
        Index_t irow = RowForCellInBox(boxIndex, i);
        Index_t icol = ColForCellInBox(boxIndex, i);
        if (irow != row && icol != col &&
                std::find(ret.data(), ret.data() + j, std::make_pair(irow, icol)) ==  ret.data() + j) {
            ret[j++] = std::make_pair(irow, icol);
        }
    }

    assert(j == NUM_BUDDIES);
    return ret;
}

bool Sudoku::IsUnique()
{
    if (boost::logic::indeterminate(_unique)) {
        Log(Trace, "uniqueness has not been determined yet, bifurcating to determine\n");
        Sudoku sudoku(*this);
        if (Bifurcate(sudoku) == 1) {
            Log(Trace, "determined puzzle to be unique\n");
            _unique = true;
        } else {
            Log(Trace, "determined puzzle to be non-unique\n");
            _unique = false;
        }
    }

    return _unique;
}


bool IsBuddy(Index_t row1, Index_t col1, Index_t row2, Index_t col2)
{
    if (row1 == row2)
        return true;
    if (col1 == col2)
        return true;
    if (BoxIndex(row1, col1) == BoxIndex(row2, col2))
        return true;
    return false;
}




unsigned Sudoku::Solve(const std::vector<Technique> &techniques)
{
    while (!IsFutileToContinue()) {
        if (TryAllTechniques(*this, techniques))
            continue;

        if (UseBifurcation() || InBifurcation())
            return Bifurcate(*this);
        else
            return 0;
    }

    if (IsSolved())
        return 1;
    else
        return 0;
}

/**
 * Checks whether it is even remotely possible to solve this puzzle
 */
bool Sudoku::IsFutileToContinue()
{
    if (AllCellsHaveValues(*this))
        return true;

    if (AnyCellsBlank(*this))
        return true;

    return false;
}

/**
 * Checks that every row, column, and box has all 9 possible numbers in it.
 */
bool Sudoku::IsSolved()
{
    for (Index_t idx = 0; idx < 9; ++idx) {
        House house = GetRow(idx);
        if (!AreAllValuesInHouse(house))
            return false;

        house = GetCol(idx);
        if (!AreAllValuesInHouse(house))
            return false;

        house = GetBox(idx);
        if (!AreAllValuesInHouse(house))
            return false;
    }
    return true;
}






void Sudoku::Output(std::ostream &out, Format fmt) const
{
    switch (fmt) {
        case Value:
            OutputByValue(*this, out);
            return;
        case Candidates:
            OutputByCandidates(*this, out);
            return;
        default:
            return;
    }
}

/**
 * @return true if the sudoku was read all the way in. false otherwise.
 */
bool Sudoku::Input(std::istream &in, Format fmt)
{
    switch (fmt) {
        case Value:
            return InputByValue(*this, in);
        case Candidates:
            return InputByCandidates(*this, in);
        case None:
            return false;
        default:
            return false;
    }
}

namespace {

void OutputByValue(const Sudoku &sudoku, std::ostream &out)
{
    for (Index_t i = 0; i < 9; ++i) {
        if (i == 3 || i == 6)
            out << '\n';

        for (Index_t j = 0; j < 9; ++j) {
            if (j == 3 || j == 6)
                out << ' ';

            Cell cell = sudoku.GetCell(i, j);

            if (cell.HasValue()) {
                out << cell.GetValue();
            } else {
                out << '.';
            }
        }

        out << '\n';
    }
}

Index_t GetMaxNumCandidatesInColumn(const Sudoku &sudoku, Index_t col)
{
    Index_t tmp = 1;
    for (Index_t i = 0; i < 9; ++i)
        tmp = std::max(tmp, sudoku.GetCell(i, col).NumCandidates());
    return tmp;
}

boost::array<Index_t, 9> GetMaxNumCandidatesAllColumns(const Sudoku &sudoku)
{
    boost::array<Index_t, 9> tmp;
    for (Index_t i = 0; i < 9; ++i)
        tmp[i] = GetMaxNumCandidatesInColumn(sudoku, i);
    return tmp;
}

void PrintLineSeparator(const boost::array<Index_t, 9> &widths, std::ostream &out)
{
    for (Index_t i = 0; i < 9; ++i)
    {
        // separate boxes every 3 cols
        if (i%3 == 0)
            out << "+-";

        for (Index_t j = 0; j < widths[i] + 1; ++j)
            out << '-';
    }
    out << "+\n";
}

void OutputByCandidates(const Sudoku &sudoku, std::ostream &out)
{
    const boost::array<Index_t, 9> widths = GetMaxNumCandidatesAllColumns(sudoku);

    for (Index_t i = 0; i < 9; ++i) {
        // separate boxes every 3 rows
        if (i%3 == 0)
            PrintLineSeparator(widths, out);

        for (Index_t j = 0; j < 9; ++j) {
            // separate boxes every 3 cols
            if (j%3 == 0)
                out << "| ";

            unsigned charsput = 0;
            Cell cell = sudoku.GetCell(i, j);
            if (cell.HasValue()) {
                out << cell.GetValue();
                ++charsput;
            } else {
                for (Index_t val = 1; val <= 9; ++val) {
                    if (cell.IsCandidate(val)) {
                        out << val;
                        ++charsput;
                    }
                }
            }

            for (; charsput < static_cast<unsigned>(widths[j] + 1); ++charsput)
                out << ' ';
        }
        out << "|\n";
    }
    PrintLineSeparator(widths, out);
}

bool InputByValue(Sudoku &sudoku, std::istream &in)
{
    sudoku.Reset();

    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            char tmp = in.get();
            while (isspace(tmp) && !in.eof())
                in.get(tmp);

            if (in.eof()) {
                Log(Info, "End of file reached while reading a sudoku, exiting...\n");
                return false;
            }

            // interpret valid values as the value to be set, everything else
            // is just an empty cell
            if (isdigit(tmp)) {
                Cell cell(static_cast<unsigned char>(tmp - '0'));
                sudoku.SetCell(cell, i, j);
            }
        }
    }

    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            if (sudoku.GetCell(i, j).HasValue())
                sudoku.CrossHatch(i, j);
        }
    }
    return true;
}

bool InputByCandidates(Sudoku &sudoku, std::istream &in)
{
    sudoku.Reset();

    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            char tmp = in.get();
            while (!isdigit(tmp) && !in.eof())
                in.get(tmp);

            if (in.eof()) {
                Log(Info, "End of file reached while reading a sudoku, exiting...\n");
                return false;
            }

            std::vector<Index_t> values;
            values.reserve(9);
            while (isdigit(tmp) && !in.eof()) {
                values.push_back(tmp - '0');
                in.get(tmp);
            }

            Cell cell;
            if (values.size() == 1) {
                cell.SetValue(values.front());
            } else {
                for (Index_t val = 1; val <= 9; ++val) {
                    if (std::find(values.begin(), values.end(), val) == values.end())
                        cell.ExcludeCandidate(val);
                }
            }
            sudoku.SetCell(cell, i, j);
        }
    }
    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            if (sudoku.GetCell(i, j).HasValue())
                sudoku.CrossHatch(i, j);
        }
    }
    return true;
}


bool IsValueInHouse(const House &house, Index_t val)
{
    for (Index_t i = 0; i < 9; ++i) {
        if (house[i].HasValue() && house[i].GetValue() == val)
            return true;
    }
    return false;
}

bool AreAllValuesInHouse(const House &house)
{
    for (Index_t val = 1; val <= 9; ++val) {
        if (!IsValueInHouse(house, val))
            return false;
    }
    return true;
}

/**
 * @return true if a technique succeeded, false if none did.
 */
bool TryAllTechniques(Sudoku &sudoku, const std::vector<Technique> &techniques)
{
    for (std::vector<Technique>::const_iterator i = techniques.begin();
            i != techniques.end(); ++i) {
        if ((*i)(sudoku))
            return true;
    }
    return false;
}

bool AllCellsHaveValues(const Sudoku &sudoku)
{
    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            if (!sudoku.GetCell(i,j).HasValue())
                return false;
        }
    }
    return true;
}

/**
 * @return true if any cells have no candidates  but are not assigned a value.
 */
bool AnyCellsBlank(const Sudoku &sudoku)
{
    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            Cell cell = sudoku.GetCell(i, j);
            if (!cell.HasValue() && cell.NumCandidates() == 0)
                return true;
        }
    }
    return false;
}

}
