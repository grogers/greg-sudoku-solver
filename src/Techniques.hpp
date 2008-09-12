#ifndef TECHNIQUES_HPP
#define TECHNIQUES_HPP

#include <boost/function.hpp>
#include <vector>

class Sudoku;

typedef boost::function<bool (Sudoku &)> Technique;

unsigned Bifurcate(Sudoku &, const std::vector<Technique> &);

bool NakedSingle(Sudoku &);
bool HiddenSingle(Sudoku &);
bool IntersectionRemoval(Sudoku &);
bool NakedSet(Sudoku &);
bool HiddenSet(Sudoku &);


#endif
