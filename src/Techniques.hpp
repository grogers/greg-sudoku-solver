#ifndef TECHNIQUES_HPP
#define TECHNIQUES_HPP

#include <boost/function.hpp>
#include <vector>

class Sudoku;

typedef boost::function<bool (Sudoku &)> Technique;

unsigned Bifurcate(Sudoku &, const std::vector<Technique> &);

bool NakedSingle(Sudoku &);
bool HiddenSingle(Sudoku &);



#endif
