#ifndef SUDOKU_HPP
#define SUDOKU_HPP

#include "Cell.hpp"
#include "Techniques.hpp"
#include "DefinePair.hpp"

#include <boost/array.hpp>
#include <vector>
#include <iosfwd>
#include <utility>
#include <boost/logic/tribool.hpp>
#include <boost/scoped_ptr.hpp>

typedef boost::array<Cell, 9> House;

const Index_t NUM_BUDDIES = 20;
DEFINE_PAIR(Position, Index_t, Index_t, row, col);

class Sudoku
{
    public:
        Sudoku();
        Sudoku(const Sudoku &);
        Sudoku &operator=(const Sudoku &);

        void Reset();

        Cell GetCell(Index_t row, Index_t col) const;
        Cell GetCell(const Position &) const;
        void SetCell(const Cell &, Index_t row, Index_t col);
        void SetCell(const Cell &, const Position &);

        void CrossHatch(Index_t row, Index_t col);
        void CrossHatch(const Position &);

        House GetRow(Index_t row) const;
        void SetRow(const House &, Index_t row);

        House GetCol(Index_t col) const;
        void SetCol(const House &, Index_t col);

        House GetBox(Index_t box) const;
        void SetBox(const House &, Index_t box);

        boost::array<Position, NUM_BUDDIES> GetBuddies(Index_t, Index_t) const;
        boost::array<Position, NUM_BUDDIES> GetBuddies(const Position &) const;

        bool IsUnique();

        unsigned Solve(const std::vector<Technique> &, bool useBifurcation);

        bool IsFutileToContinue();
        bool IsSolved();

        enum Format
        {
            Value,
            Candidates,
            SingleLine,
            None
        };

        void Output(std::ostream &, Format = Candidates) const;
        bool Input(std::istream &, Format = Value);

    private:
        // first index is for row, second index is for column
        boost::array<boost::array<Cell, 9>, 9> _board;
        // if something is unique, special techniques can be used
        boost::logic::tribool _unique;
        boost::scoped_ptr<boost::array<boost::array<Cell, 9>, 9> > _uniquely_solved_board;
};

bool IsBuddy(Index_t row1, Index_t col1, Index_t row2, Index_t col2);
inline bool IsBuddy(const Position &cell1, const Position &cell2)
{
    return IsBuddy(cell1.row, cell1.col, cell2.row, cell2.col);
}

inline Sudoku::Sudoku()
{
    Reset();
}

inline Sudoku::Sudoku(const Sudoku &x)
{
    *this = x;
}

inline Cell Sudoku::GetCell(Index_t row, Index_t col) const
{
    return _board[row][col];
}

inline Cell Sudoku::GetCell(const Position &x) const
{
    return _board[x.row][x.col];
}

inline void Sudoku::SetCell(const Cell &cell, Index_t row, Index_t col)
{
    _board[row][col] = cell;
}

inline void Sudoku::SetCell(const Cell &cell, const Position &x)
{
    _board[x.row][x.col] = cell;
}

inline boost::array<Position, NUM_BUDDIES>
Sudoku::GetBuddies(const Position &x) const
{
    return GetBuddies(x.row, x.col);
}


inline Index_t BoxIndex(Index_t row, Index_t col)
{
    return (row/3)*3 + col/3;
}

inline Index_t RowForCellInBox(Index_t box, Index_t pos)
{
    return (box/3)*3 + pos/3;
}

inline Index_t ColForCellInBox(Index_t box, Index_t pos)
{
    return (box%3)*3 + pos%3;
}

inline Position CellInBox(Index_t box, Index_t pos)
{
    return Position(RowForCellInBox(box, pos), ColForCellInBox(box, pos));
}


#endif
