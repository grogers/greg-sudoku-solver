#include "Sudoku.hpp"
#include "Logging.hpp"
#include "DefinePair.hpp"

#include <climits>
#include <map>
#include <set>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace {
bool SimpleColorForValue(Sudoku &, Index_t);
DEFINE_PAIR(Color, Index_t, bool, id, parity);
Color FlipColor(const Color &x);
typedef std::map<std::pair<Index_t, Index_t>, Color> ColorMap;
void BuildColorMap(const Sudoku &, Index_t, ColorMap &);
bool BilocationInHouse(const House &, Index_t, Index_t &, Index_t &);
void AddConjugateCellsToColorMap(const std::pair<Index_t, Index_t> &,
        const std::pair<Index_t, Index_t> &, ColorMap &, Index_t);
void PrintColorMap(const ColorMap &, Index_t);
bool SimpleColorEliminations(Sudoku &sudoku, const ColorMap &, Index_t);
bool EliminateCellsWhichSeeBothConjugates(Sudoku &, const ColorMap &, Index_t);
bool EliminateColorSeesItself(Sudoku &, const ColorMap &, Index_t);
bool EliminateColorSeesAllCellsInHouse(Sudoku &, const ColorMap &, Index_t);
std::set<Color> BuildSetOfColors(const ColorMap &);
std::set<std::pair<Index_t, Index_t> > BuildColorCoverage(const Sudoku &,
        const ColorMap &, const Color &, Index_t);
}


bool SimpleColor(Sudoku &sudoku)
{
    Log(Trace, "searching for simple color eliminations\n");

    for (Index_t val = 1; val <= 9; ++val) {
        if (SimpleColorForValue(sudoku, val))
            return true;
    }
    return false;
}


namespace {

Color ParityFlipped(const Color &x)
{
    Color ret(x);
    ret.parity = !ret.parity;
    return ret;
}

bool SimpleColorForValue(Sudoku &sudoku, Index_t value)
{
    ColorMap colors;
    BuildColorMap(sudoku, value, colors);
    return SimpleColorEliminations(sudoku, colors, value);
}

void BuildColorMap(const Sudoku &sudoku, Index_t value, ColorMap &colors)
{
    Index_t cnt = 0;
    for (Index_t i = 0; i < 9; ++i) {
        Index_t idx1, idx2;
        if (BilocationInHouse(sudoku.GetRow(i), value, idx1, idx2)) {
            AddConjugateCellsToColorMap(std::make_pair(i, idx1),
                    std::make_pair(i, idx2), colors, cnt++);
        }
        if (BilocationInHouse(sudoku.GetCol(i), value, idx1, idx2)) {
            AddConjugateCellsToColorMap(std::make_pair(idx1, i),
                    std::make_pair(idx2, i), colors, cnt++);
        }
        if (BilocationInHouse(sudoku.GetBox(i), value, idx1, idx2)) {
            AddConjugateCellsToColorMap(CellInBox(i, idx1), CellInBox(i, idx2),
                    colors, cnt++);
        }
    }
}

bool BilocationInHouse(const House &house, Index_t value, Index_t &idx1,
        Index_t &idx2)
{
    Index_t cnt = 0;
    for (Index_t i = 0; i < 9; ++i) {
        if (house[i].IsCandidate(value)) {
            switch (cnt++) {
                case 0:
                    idx1 = i;
                    break;
                case 1:
                    idx2 = i;
                    break;
                default:
                    return false;
            }
        }
    }
    return cnt == 2;
}

void AddConjugateCellsToColorMap(const std::pair<Index_t, Index_t> &cell1,
        const std::pair<Index_t, Index_t> &cell2, ColorMap &colors,
        Index_t colorId)
{
    ColorMap::iterator it1 = colors.find(cell1);
    ColorMap::iterator it2 = colors.find(cell2);

    if (it1 == colors.end()) {
        if (it2 == colors.end()) {
            colors[cell1] = Color(colorId, false);
            colors[cell2] = Color(colorId, true);
        } else {
            colors[cell1] = ParityFlipped(it2->second);
        }
    } else {
        if (it2 == colors.end()) {
            colors[cell2] = ParityFlipped(it1->second);
        } else {
            // replace the colors at cell2 with the conjugate of the colors at
            // cell 1
            Index_t newColorId = it1->second.id;
            Index_t oldColorId = it2->second.id;
            bool flipParity = (it1->second.parity == it2->second.parity);
            for (ColorMap::iterator i = colors.begin(); i != colors.end(); ++i) {
                if (i->second.id == oldColorId) {
                    i->second = Color(newColorId,
                            (i->second.parity != flipParity));
                }
            }
        }
    }
}

void PrintColorMap(const ColorMap &colors, Index_t value)
{
    std::cout << "color map for value " << value << ":\n";
    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            ColorMap::const_iterator it = colors.find(std::make_pair(i, j));
            if (it == colors.end()) {
                std::cout << "   ";
            } else {
                std::cout << it->second.id;
                if (it->second.parity)
                    std::cout << '+';
                else
                    std::cout << '-';
                std::cout << ' ';
            }
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

bool SimpleColorEliminations(Sudoku &sudoku, const ColorMap &colors,
        Index_t value)
{
    bool ret = false;
    if (EliminateCellsWhichSeeBothConjugates(sudoku, colors, value))
        ret = true;
    if (EliminateColorSeesItself(sudoku, colors, value))
        ret = true;
    if (EliminateColorSeesAllCellsInHouse(sudoku, colors, value))
        ret = true;
    return ret;
}

bool EliminateCellsWhichSeeBothConjugates(Sudoku &sudoku,
        const ColorMap &colors, Index_t value)
{
    bool ret = false;
    std::vector<std::pair<Index_t, Index_t> > changed;
    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            if (!sudoku.GetCell(i, j).IsCandidate(value) ||
                    colors.find(std::make_pair(i, j)) != colors.end())
                continue;

            boost::array<std::pair<Index_t, Index_t>, NUM_BUDDIES> buddies =
                sudoku.GetBuddies(i, j);

            std::set<Color> colorsSeen;
            for (Index_t k = 0; k < NUM_BUDDIES; ++k) {
                ColorMap::const_iterator it = colors.find(buddies[k]);
                if (it != colors.end())
                    colorsSeen.insert(it->second);
            }

            for (std::set<Color>::const_iterator it =
                    colorsSeen.begin(); it != colorsSeen.end(); ++it) {
                if (colorsSeen.find(ParityFlipped(*it)) != colorsSeen.end()) {
                    Cell cell = sudoku.GetCell(i, j);
                    if (cell.ExcludeCandidate(value)) {
                        sudoku.SetCell(cell, i, j);
                        changed.push_back(std::make_pair(i, j));
                        ret = true;
                    }
                }
            }
        }
    }

    if (ret) {
        std::ostringstream sstr;
        for (Index_t i = 0; i != changed.size(); ++i) {
            if (i != 0)
                sstr << ", ";
            sstr << 'r' << changed[i].first+1 << 'c' << changed[i].first+1
                << '#' << value;
        }

        Log(Info, "simple colors (cell sees both colors) ==> %s\n",
                sstr.str().c_str());
    }

    return ret;
}

bool EliminateColorSeesItself(Sudoku &sudoku, const ColorMap &colors,
        Index_t value)
{
    for (ColorMap::const_iterator i = colors.begin(); i != colors.end(); ++i) {
        Index_t row = i->first.first, col = i->first.second;
        Color currColor = i->second;

        ColorMap::const_iterator j = i;
        for (++j; j != colors.end(); ++j) {
            if (currColor != j->second)
                continue;

            if (IsBuddy(row, col, j->first.first, j->first.second)) {
                std::vector<std::pair<Index_t, Index_t> > changed;
                for (ColorMap::const_iterator k = colors.begin();
                        k != colors.end(); ++k) {
                    if (k->second == currColor) {
                        Index_t r = k->first.first, c = k->first.second;
                        Cell cell = sudoku.GetCell(r, c);
                        if (cell.ExcludeCandidate(value)) {
                            sudoku.SetCell(cell, r, c);
                            changed.push_back(std::make_pair(r, c));
                        }
                    }
                }

                assert(changed.size() > 0);

                std::ostringstream sstr;
                for (Index_t k = 0; k != changed.size(); ++k) {
                    if (k != 0)
                        sstr << ", ";
                    sstr << 'r' << changed[k].first+1 << 'c'
                        << changed[k].second+1 << '#' << value;
                }

                Log(Info, "simple colors (color sees itself) ==> %s\n",
                        sstr.str().c_str());
                return true;
            }
        }
    }
    return false;
}

bool EliminateColorSeesAllCellsInHouse(Sudoku &sudoku, const ColorMap &colors,
        Index_t value)
{
    std::set<Color> colorSet = BuildSetOfColors(colors);

    for (std::set<Color>::const_iterator it = colorSet.begin();
            it != colorSet.end(); ++it) {
        for (Index_t i = 0; i < 9; ++i) {
            House house = sudoku.GetRow(i);
        }
    }

    return false;
}

std::set<Color> BuildSetOfColors(const ColorMap &colors)
{
    std::set<Color> ret;
    for (ColorMap::const_iterator i = colors.begin(); i != colors.end(); ++i) {
        ret.insert(i->second);
    }
    return ret;
}

/**
 * when executing this procedure, we assume that we have already found colors
 * that see each other, so no buddies of any cell of a certain color will lie
 * on that color.
 */
std::set<std::pair<Index_t, Index_t> >
BuildColorCoverage(const Sudoku &sudoku, const ColorMap &colorMap,
        const Color &color)
{
    std::set<std::pair<Index_t, Index_t> > ret;
    typedef ColorMap::const_iterator CIter;
    for (CIter i = colorMap.begin(); i != colorMap.end(); ++i) {
        if (i->second != color)
            continue;

        boost::array<std::pair<Index_t, Index_t>, NUM_BUDDIES> buddies =
            sudoku.GetBuddies(i->first.first, i->first.second);
        for (Index_t j = 0; j < NUM_BUDDIES; ++j)
            ret.insert(buddies[j]);
    }
    return ret;
}


}
