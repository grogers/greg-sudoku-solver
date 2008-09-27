#ifndef SUDOKU_HPP
#define SUDOKU_HPP

#include "Cell.hpp"
#include "Techniques.hpp"

#include <boost/array.hpp>
#include <vector>
#include <iosfwd>
#include <utility>
#include <boost/logic/tribool.hpp>
#include <boost/scoped_ptr.hpp>

typedef boost::array<Cell, 9> House;

const Index_t NUM_BUDDIES = 20;

class Sudoku
{
    public:
        Sudoku();
        Sudoku(const Sudoku &);
        Sudoku &operator=(const Sudoku &);

        void Reset();

        Cell GetCell(Index_t row, Index_t col) const;
        void SetCell(const Cell &, Index_t row, Index_t col);

        void CrossHatch(Index_t row, Index_t col);

        House GetRow(Index_t row) const;
        void SetRow(const House &, Index_t row);

        House GetCol(Index_t col) const;
        void SetCol(const House &, Index_t col);

        House GetBox(Index_t box) const;
        void SetBox(const House &, Index_t box);

        boost::array<std::pair<Index_t, Index_t>, NUM_BUDDIES> GetBuddies(Index_t, Index_t) const;

        bool IsUnique();

        unsigned Solve(const std::vector<Technique> &, bool useBifurcation);

        bool IsFutileToContinue();
        bool IsSolved();

        enum Format
        {
            Value,
            Candidates,
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

inline void Sudoku::SetCell(const Cell &cell, Index_t row, Index_t col)
{
    _board[row][col] = cell;
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


#endif
