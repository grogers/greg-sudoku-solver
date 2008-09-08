#ifndef SUDOKU_HPP
#define SUDOKU_HPP

#include "Cell.hpp"

#include <boost/array.hpp>

class Sudoku
{
    public:
        Sudoku();
        Sudoku(const Sudoku &);
        Sudoku &operator=(const Sudoku &);

        Cell GetCell(Index_t row, Index_t col);

    private:
        // first index is for row, second index is for column
        boost::array<boost::array<Cell, 9>, 9> _board;
};

#endif
