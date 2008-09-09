#include "Sudoku.hpp"
#include "Logging.hpp"
#include "Options.hpp"

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
    bool IsValueInHouse(const House &, Index_t);
    bool AreAllValuesInHouse(const House &);
    bool TryAllTechniques(Sudoku &, const std::vector<Technique> &);
}

Sudoku &Sudoku::operator=(const Sudoku &x)
{
    _board = x._board;
    return *this;
}

void Sudoku::Reset()
{
    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            _board[i][j] = Cell();
        }
    }
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

unsigned Sudoku::Solve(const std::vector<Technique> &techniques)
{
    while (!IsFutileToContinue()) {
        if (TryAllTechniques(*this, techniques))
            continue;

        if (UseBifurcation())
            return Bifurcate(*this, techniques);
        else
            return 0;
    }

    if (IsSolved())
        return 1;
    else
        return 0;
}

/**
 * Checks whether any more cells can possibly filled out.
 */
bool Sudoku::IsFutileToContinue()
{
    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            if (!_board[i][j].HasValue())
                return false;
        }
    }
    return true;
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
            return false;
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

            int charsput = 0;
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

            for (; charsput < widths[j] + 1; ++charsput)
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
            if (tmp >= '1' && tmp <= '9') {
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

}
