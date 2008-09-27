#ifndef TECHNIQUES_HPP
#define TECHNIQUES_HPP

#include <boost/function.hpp>
#include <vector>

class Sudoku;

typedef boost::function<bool (Sudoku &)> Technique;

unsigned Bifurcate(Sudoku &);

bool NakedSingle(Sudoku &);
bool HiddenSingle(Sudoku &);
bool IntersectionRemoval(Sudoku &);

bool NakedSet(Sudoku &);
bool HiddenSet(Sudoku &);

bool BasicFish(Sudoku &);
bool FinnedFish(Sudoku &);
bool FrankenFish(Sudoku &);
bool MutantFish(Sudoku &);

bool XyWing(Sudoku &);
bool XyzWing(Sudoku &);
bool RemotePair(Sudoku &);

bool UniqueRectangle(Sudoku &);


#endif
