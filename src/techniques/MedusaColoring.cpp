#include "Sudoku.hpp"
#include "Logging.hpp"
#include "Coloring.hpp"

#include <climits>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <functional>
#include <set>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/key_extractors.hpp>

#include <boost/tuple/tuple.hpp>

namespace mi = boost::multi_index;

namespace {

struct ColoredCandidate
{
    Position pos;
    Index_t value;
    Color color;

    ColoredCandidate() {}
    ColoredCandidate(const ColoredCandidate &x) :
        pos(x.pos), value(x.value), color(x.color) {}
    ColoredCandidate(const Position &pos, Index_t value,
            Color color = Color()) :
        pos(pos), value(value), color(color) {}
    ColoredCandidate(Index_t row, Index_t col, Index_t value,
            Color color = Color()) :
        pos(Position(row, col)), value(value), color(color) {}
};

struct PositionValueComp :
    public std::binary_function<ColoredCandidate, ColoredCandidate, bool>
{
    bool operator()(const ColoredCandidate &c1, const ColoredCandidate &c2) const
    {
        return c1.pos < c2.pos || (!(c2.pos < c1.pos) && c1.value < c2.value);
    }
};

typedef mi::multi_index_container<
    ColoredCandidate,
    mi::indexed_by<
        mi::ordered_unique<
            mi::identity<ColoredCandidate>, PositionValueComp
        >,
        mi::ordered_non_unique<
            mi::member<ColoredCandidate, Color, &ColoredCandidate::color>
        >
    >
> ColorContainer;

ColorContainer BuildMedusaColors(const Sudoku &);
boost::tuple<bool, Index_t, Index_t> FindBilocation(const House &, Index_t);
boost::tuple<bool, Index_t, Index_t> FindBivalue(const Cell &);
void AddConjugates(ColorContainer &, const Position &, Index_t,
        const Position &, Index_t);
void PrintColorContainer(const ColorContainer &);
bool MedusaColorEliminations(Sudoku &sudoku, const ColorContainer &);
bool EliminateCandidatesThatSeeConjugates(Sudoku &, const ColorContainer &);
std::set<Color> BuildColorsCandidateCanSee(const Sudoku &,
        const ColorContainer &, const ColoredCandidate &);
bool EliminateColorThatSeesItself(Sudoku &, const ColorContainer &);

std::string ChangedCandidatesToString(const std::vector<ColoredCandidate> &);
}


bool MedusaColor(Sudoku &sudoku)
{
    Log(Trace, "searching for 3d medusa color eliminations\n");

    ColorContainer colors = BuildMedusaColors(sudoku);

    /*
    sudoku.Output(std::cout);
    PrintColorContainer(colors);
    */

    return MedusaColorEliminations(sudoku, colors);
}


namespace {

ColorContainer BuildMedusaColors(const Sudoku &sudoku)
{
    ColorContainer ret;

    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t val = 1; val <= 9; ++val) {
            bool found;
            Index_t j1, j2;

            boost::tie(found, j1, j2) = FindBilocation(sudoku.GetRow(i), val);
            if (found)
                AddConjugates(ret, Position(i, j1), val, Position(i, j2), val);

            boost::tie(found, j1, j2) = FindBilocation(sudoku.GetCol(i), val);
            if (found)
                AddConjugates(ret, Position(j1, i), val, Position(j2, i), val);

            boost::tie(found, j1, j2) = FindBilocation(sudoku.GetBox(i), val);
            if (found)
                AddConjugates(ret, CellInBox(i, j1), val, CellInBox(i, j2), val);
        }
    }

    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            bool found;
            Index_t val1, val2;

            boost::tie(found, val1, val2) = FindBivalue(sudoku.GetCell(i, j));
            if (found)
                AddConjugates(ret, Position(i, j), val1, Position(i, j), val2);
        }
    }

    return ret;
}

boost::tuple<bool, Index_t, Index_t>
    FindBilocation(const House &house, Index_t value)
{
    boost::tuple<bool, Index_t, Index_t> ret;
    Index_t cnt = 0;
    for (Index_t i = 0; i < 9; ++i) {
        if (house[i].IsCandidate(value)) {
            switch (cnt++) {
                case 0:
                    ret.get<1>() = i;
                    break;
                case 1:
                    ret.get<2>() = i;
                    break;
                default:
                    ret.get<0>() = false;
                    return ret;
            }
        }
    }
    ret.get<0>() = (cnt == 2);
    return ret;
}

boost::tuple<bool, Index_t, Index_t> FindBivalue(const Cell &cell)
{
    boost::tuple<bool, Index_t, Index_t> ret;
    Index_t cnt = 0;
    for (Index_t val = 1; val <= 9; ++val) {
        if (cell.IsCandidate(val)) {
            switch (cnt++) {
                case 0:
                    ret.get<1>() = val;
                    break;
                case 1:
                    ret.get<2>() = val;
                    break;
                default:
                    ret.get<0>() = false;
                    return ret;
            }
        }
    }
    ret.get<0>() = (cnt == 2);
    return ret;
}

void AddConjugates(ColorContainer &colors, const Position &pos1, Index_t val1,
        const Position &pos2, Index_t val2)
{
    Index_t colorId = 0;
    ColorContainer::nth_index<1>::type &colorView = colors.get<1>();
    if (colorView.rbegin() != colorView.rend()) {
        colorId = (++colorView.rbegin())->color.id + 1;
    }

    const ColorContainer::nth_index<0>::type &posView = colors.get<0>();
    ColorContainer::nth_index<0>::type::iterator
        it1 = posView.find(ColoredCandidate(pos1, val1)),
        it2 = posView.find(ColoredCandidate(pos2, val2));

    if (it1 == posView.end()) {
        if (it2 == posView.end()) {
            colors.insert(
                    ColoredCandidate(pos1, val1, Color(colorId, false)));
            colors.insert(
                    ColoredCandidate(pos2, val2, Color(colorId, true)));
        } else {
            colors.insert(
                    ColoredCandidate(pos1, val1, ParityFlipped(it2->color)));
        }
    } else {
        if (it2 == posView.end()) {
            colors.insert(
                    ColoredCandidate(pos2, val2, ParityFlipped(it1->color)));
        } else {
            Index_t newColorId = it1->color.id;
            Index_t oldColorId = it2->color.id;

            if (oldColorId != newColorId) {
                bool flipParity = (it1->color.parity == it2->color.parity);
                typedef ColorContainer::nth_index<1>::type::iterator Iter;
                // color parity orders false before true for operator<
                Iter first = colorView.lower_bound(Color(oldColorId, false));
                Iter last = colorView.upper_bound(Color(oldColorId, true));

                for (Iter i = first; i != last;) {
                    ColoredCandidate tmpCand(i->pos, i->value,
                            Color(newColorId, (i->color.parity != flipParity)));
                    Iter tmp = i++;
                    colorView.erase(tmp);
                    colors.insert(tmpCand);
                }
            }
        }
    }
}

void PrintColorContainer(const ColorContainer &colors)
{
    std::cout << "3D Colors:\n";
    const ColorContainer::nth_index<0>::type &posView = colors.get<0>();

    for (Index_t val = 1; val <= 9; ++val) {
        std::cout << "value " << val << ":\n";
        for (Index_t i = 0; i < 9; ++i) {
            if (i%3 == 0 && i != 0)
                std::cout << "------------+-------------+------------\n";
            for (Index_t j = 0; j < 9; ++j) {
                if (j%3 == 0 && j != 0)
                    std::cout << "| ";

                ColorContainer::nth_index<0>::type::iterator it =
                    posView.find(ColoredCandidate(i, j, val));
                if (it == posView.end()) {
                    std::cout << "    ";
                } else {
                    std::cout << std::setw(2) << it->color.id;
                    std::cout << ((it->color.parity) ? '+' : '-') << ' ';
                }
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }
}

bool MedusaColorEliminations(Sudoku &sudoku, const ColorContainer &colors)
{
    bool ret = false;
    if (EliminateCandidatesThatSeeConjugates(sudoku, colors))
        ret = true;
    if (EliminateColorThatSeesItself(sudoku, colors))
        ret = true;
    return ret;
}

bool EliminateCandidatesThatSeeConjugates(Sudoku &sudoku,
        const ColorContainer &colors)
{
    bool ret = false;
    std::vector<ColoredCandidate> changed;
    const ColorContainer::nth_index<0>::type &posView = colors.get<0>();

    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            if (sudoku.GetCell(i, j).HasValue())
                continue;

            std::vector<Index_t> values =
                CandidatesForCell(sudoku.GetCell(i, j));

            for (Index_t k = 0; k < values.size(); ++k) {
                ColoredCandidate cand(i, j, values[k]);
                if (posView.find(cand) != posView.end())
                    continue;

                std::set<Color> colorsSeen =
                    BuildColorsCandidateCanSee(sudoku, colors, cand);

                for (std::set<Color>::const_iterator it = colorsSeen.begin();
                        it != colorsSeen.end(); ++it) {
                    if (colorsSeen.find(ParityFlipped(*it)) !=
                            colorsSeen.end()) {
                        Cell cell = sudoku.GetCell(cand.pos);
                        if (cell.ExcludeCandidate(cand.value)) {
                            sudoku.SetCell(cell, cand.pos);
                            changed.push_back(cand);
                            ret = true;
                        }
                    }
                }
            }
        }
    }

    if (ret) {
        std::string str = ChangedCandidatesToString(changed);
        Log(Info, "3d medusa colors (candidate sees both colors) ==> %s\n",
                str.c_str());
    }

    return ret;
}

std::set<Color> BuildColorsCandidateCanSee(const Sudoku &sudoku,
        const ColorContainer &colors,
        const ColoredCandidate &cand)
{
    std::set<Color> ret;

    const ColorContainer::nth_index<0>::type &posView = colors.get<0>();

    std::vector<Index_t> values = CandidatesForCell(sudoku.GetCell(cand.pos));
    for (Index_t i = 0; i < values.size(); ++i) {
        if (values[i] == cand.value)
            continue;

        ColorContainer::nth_index<0>::type::iterator it =
            posView.find(ColoredCandidate(cand.pos, values[i]));
        if (it != posView.end())
            ret.insert(it->color);
    }

    boost::array<Position, NUM_BUDDIES> buddies = sudoku.GetBuddies(cand.pos);
    for (Index_t i = 0; i < NUM_BUDDIES; ++i) {
        if (!sudoku.GetCell(buddies[i]).IsCandidate(cand.value))
            continue;

        ColorContainer::nth_index<0>::type::iterator it =
            posView.find(ColoredCandidate(buddies[i], cand.value));
        if (it != posView.end())
            ret.insert(it->color);
    }

    return ret;
}

std::string ChangedCandidatesToString(const std::vector<ColoredCandidate> &changed)
{
    std::ostringstream sstr;
    for (Index_t i = 0; i != changed.size(); ++i) {
        if (i != 0)
            sstr << ", ";
        sstr << 'r' << changed[i].pos.row+1 << 'c'
            << changed[i].pos.col+1 << '#' << changed[i].value;
    }
    return sstr.str();
}

bool EliminateColorThatSeesItself(Sudoku &sudoku, const ColorContainer &colors)
{
    bool ret = false;

    return ret;
}

}
