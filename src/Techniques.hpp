#ifndef TECHNIQUES_HPP
#define TECHNIQUES_HPP

#include <boost/function.hpp>
#include <vector>

class Sudoku;

typedef boost::function<bool (Sudoku &)> Technique;

unsigned Bifurcate(Sudoku &);

bool NakedSingle(Sudoku &);
bool NakedPair(Sudoku &);
bool NakedTriple(Sudoku &);
bool NakedQuad(Sudoku &);

bool HiddenSingle(Sudoku &);
bool HiddenPair(Sudoku &);
bool HiddenTriple(Sudoku &);
bool HiddenQuad(Sudoku &);

bool LockedCandidates(Sudoku &);

bool XWing(Sudoku &);
bool Swordfish(Sudoku &);
bool Jellyfish(Sudoku &);

bool SimpleColor(Sudoku &);
bool MultiColor(Sudoku &);
bool MedusaColor(Sudoku &);

bool XyWing(Sudoku &);
bool XyzWing(Sudoku &);
bool RemotePair(Sudoku &);

bool SimpleSudokuTechniqueSet(Sudoku &);

bool UniqueRectangle(Sudoku &);

bool FinnedFish(Sudoku &);
bool FrankenFish(Sudoku &);
bool MutantFish(Sudoku &);



#endif
