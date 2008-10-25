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
typedef ColorContainer::nth_index<0>::type PositionView;
typedef ColorContainer::nth_index<1>::type ColorView;

ColorContainer BuildMedusaColors(const Sudoku &);
boost::tuple<bool, Index_t, Index_t> FindBilocation(const House &, Index_t);
boost::tuple<bool, Index_t, Index_t> FindBivalue(const Cell &);
void AddConjugates(ColorContainer &, const Position &, Index_t,
        const Position &, Index_t);
void PrintColorContainer(const PositionView &);
bool MedusaColorEliminations(Sudoku &sudoku, const ColorContainer &);
bool EliminateCandidatesThatSeeConjugates(Sudoku &, const PositionView &);
std::set<Color> BuildColorsCandidateCanSee(const Sudoku &, const PositionView &,
        const ColoredCandidate &);
bool EliminateColorsThatSeeThemselves(Sudoku &, const ColorView &);
std::string ChangedCandidatesToString(const std::vector<ColoredCandidate> &);
std::set<Color> GetColorsInContainer(const ColorView &);
bool ColorSeesItself(const ColorView &, const Color &);
bool IsWeaklyLinked(const ColoredCandidate &, const ColoredCandidate &);
void RemoveColor(Sudoku &, const ColorView &, const Color &,
        std::vector<ColoredCandidate> &);
bool EliminateColorsThatSeeConjugates(Sudoku &, const ColorContainer &);
std::set<ColoredCandidate, PositionValueComp> BuildColorCoverage(const Sudoku &,
        const ColorContainer &, const Color &);
bool AnyConjugateColorInCoverage(const PositionView &,
        const std::set<ColoredCandidate, PositionValueComp> &);
bool EliminateCandidatesThatSeeColorWing(Sudoku &sudoku, const ColorContainer &);
std::set<std::pair<Color, Color> > BuildColorLinks(const ColorContainer &);
std::string ColorToString(const Color &);
}


bool MedusaColor(Sudoku &sudoku)
{
    Log(Trace, "searching for 3d medusa color eliminations\n");

    ColorContainer colors = BuildMedusaColors(sudoku);
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
    ColorView &colorView = colors.get<1>();
    if (colorView.rbegin() != colorView.rend()) {
        colorId = (++colorView.rbegin())->color.id + 1;
    }

    const PositionView &posView = colors.get<0>();
    PositionView::iterator
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
                // color parity orders false before true for operator<
                ColorView::iterator first =
                    colorView.lower_bound(Color(oldColorId, false));
                ColorView::iterator last =
                    colorView.upper_bound(Color(oldColorId, true));

                for (ColorView::iterator i = first; i != last;) {
                    ColoredCandidate tmpCand(i->pos, i->value,
                            Color(newColorId, (i->color.parity != flipParity)));
                    ColorView::iterator tmp = i++;
                    colorView.erase(tmp);
                    colors.insert(tmpCand);
                }
            }
        }
    }
}

void PrintColorContainer(const PositionView &posView)
{
    std::cout << "3D Colors:\n";
    for (Index_t val = 1; val <= 9; ++val) {
        std::cout << "value " << val << ":\n";
        for (Index_t i = 0; i < 9; ++i) {
            if (i%3 == 0 && i != 0)
                std::cout << "------------+-------------+------------\n";
            for (Index_t j = 0; j < 9; ++j) {
                if (j%3 == 0 && j != 0)
                    std::cout << "| ";

                PositionView::iterator it =
                    posView.find(ColoredCandidate(i, j, val));
                if (it == posView.end())
                    std::cout << "    ";
                else
                    std::cout << ColorToString(it->color) << ' ';
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }
}

bool MedusaColorEliminations(Sudoku &sudoku, const ColorContainer &colors)
{
    if (EliminateCandidatesThatSeeConjugates(sudoku, colors.get<0>()))
        return true;
    if (EliminateColorsThatSeeThemselves(sudoku, colors.get<1>()))
        return true;
    //EliminateColorThatSeesAllCellsInHouse
    //EliminateColorThatSeesAllCandidatesInCell
    if (EliminateColorsThatSeeConjugates(sudoku, colors))
        return true;
    if (EliminateCandidatesThatSeeColorWing(sudoku, colors))
        return true;
    return false;
}

bool EliminateCandidatesThatSeeConjugates(Sudoku &sudoku,
        const PositionView &posView)
{
    bool ret = false;
    std::vector<ColoredCandidate> changed;

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
                    BuildColorsCandidateCanSee(sudoku, posView, cand);

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
        const PositionView &posView,
        const ColoredCandidate &cand)
{
    std::set<Color> ret;

    std::vector<Index_t> values = CandidatesForCell(sudoku.GetCell(cand.pos));
    for (Index_t i = 0; i < values.size(); ++i) {
        if (values[i] == cand.value)
            continue;

        PositionView::iterator it =
            posView.find(ColoredCandidate(cand.pos, values[i]));
        if (it != posView.end())
            ret.insert(it->color);
    }

    boost::array<Position, NUM_BUDDIES> buddies = sudoku.GetBuddies(cand.pos);
    for (Index_t i = 0; i < NUM_BUDDIES; ++i) {
        if (!sudoku.GetCell(buddies[i]).IsCandidate(cand.value))
            continue;

        PositionView::iterator it =
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

bool EliminateColorsThatSeeThemselves(Sudoku &sudoku,
        const ColorView &colorView)
{
    bool ret = false;

    std::set<Color> colorsAvail = GetColorsInContainer(colorView);
    for (std::set<Color>::iterator it = colorsAvail.begin();
            it != colorsAvail.end(); ++it) {
        if (ColorSeesItself(colorView, *it)) {
            std::vector<ColoredCandidate> changed;
            RemoveColor(sudoku, colorView, *it, changed);

            if (!changed.empty()) {
                std::string str = ChangedCandidatesToString(changed);
                Log(Info, "3d medusa colors (color sees itself) ==> %s\n",
                        str.c_str());
                ret = true;
            }
        }
    }

    return ret;
}

std::set<Color> GetColorsInContainer(const ColorView &colorView)
{
    std::set<Color> ret;
    for (ColorView::iterator i = colorView.begin(); i != colorView.end(); ++i) {
        ret.insert(i->color);
    }
    return ret;
}

bool IsWeaklyLinked(const ColoredCandidate &cand1,
        const ColoredCandidate &cand2)
{
    return (cand1.pos == cand2.pos && cand1.value != cand2.value) ||
        (IsBuddy(cand1.pos, cand2.pos) && cand1.value == cand2.value);
}

bool ColorSeesItself(const ColorView &colorView, const Color &color)
{
    ColorView::iterator first, last;
    boost::tie(first, last) = colorView.equal_range(color);

    for (ColorView::iterator i = first; i != last; ++i) {
        for (ColorView::iterator j = ++ColorView::iterator(i); j != last; ++j) {
            if (IsWeaklyLinked(*i, *j))
                return true;
        }
    }
    return false;
}

void RemoveColor(Sudoku &sudoku, const ColorView &colorView,
        const Color &color, std::vector<ColoredCandidate> &changed)
{
    ColorView::iterator first, last;
    boost::tie(first, last) = colorView.equal_range(color);

    for (ColorView::iterator i = first; i != last; ++i) {
        Cell cell = sudoku.GetCell(i->pos);
        if (cell.ExcludeCandidate(i->value)) {
            sudoku.SetCell(cell, i->pos);
            changed.push_back(*i);
        }
    }
}

bool EliminateColorsThatSeeConjugates(Sudoku &sudoku,
        const ColorContainer &colors)
{
    bool ret = false;
    const ColorView &colorView = colors.get<1>();
    std::set<Color> colorsAvail = GetColorsInContainer(colorView);

    for (std::set<Color>::iterator it = colorsAvail.begin();
            it != colorsAvail.end(); ++it) {
        std::set<ColoredCandidate, PositionValueComp> coverage =
            BuildColorCoverage(sudoku, colors, *it);

        if (AnyConjugateColorInCoverage(colors.get<0>(), coverage)) {
            std::vector<ColoredCandidate> changed;
            RemoveColor(sudoku, colorView, *it, changed);

            if (!changed.empty()) {
                std::string str = ChangedCandidatesToString(changed);
                Log(Info, "3d medusa colors (color sees conjugate colors) ==> %s\n",
                        str.c_str());
                ret = true;
            }
        }
    }

    return ret;
}

/**
 * @note we assume we have already found colors that see themselves, so no
 * buddies of any candidate will be the same color.
 *
 * @note the returned set contains ColoredCandidate's but the color of them is
 * not set (if one even exists for it)
 */
std::set<ColoredCandidate, PositionValueComp> BuildColorCoverage(
        const Sudoku &sudoku, const ColorContainer &colors, const Color &color)
{
    std::set<ColoredCandidate, PositionValueComp> ret;

    const ColorView &colorView = colors.get<1>();

    ColorView::iterator first, last;
    boost::tie(first, last) = colorView.equal_range(color);

    for (ColorView::iterator it = first; it != last; ++it) {
        std::vector<Index_t> values =
            CandidatesForCell(sudoku.GetCell(it->pos));
        for (Index_t i = 0; i < values.size(); ++i) {
            if (values[i] == it->value)
                continue;

            ret.insert(ColoredCandidate(it->pos, values[i]));
        }

        boost::array<Position, NUM_BUDDIES> buddies =
            sudoku.GetBuddies(it->pos);
        for (Index_t i = 0; i < NUM_BUDDIES; ++i) {
            if (!sudoku.GetCell(buddies[i]).IsCandidate(it->value))
                continue;

            ret.insert(ColoredCandidate(buddies[i], it->value));
        }
    }

    return ret;
}


bool AnyConjugateColorInCoverage(const PositionView &posView, const std::set<ColoredCandidate, PositionValueComp> &coverage)
{
    std::set<Color> colorsSeen;

    for (std::set<ColoredCandidate, PositionValueComp>::iterator it = coverage.begin();
            it != coverage.end(); ++it) {
        PositionView::iterator tmp = posView.find(*it);
        if (tmp != posView.end())
            colorsSeen.insert(tmp->color);
    }

    for (std::set<Color>::iterator it = colorsSeen.begin();
            it != colorsSeen.end(); ++it) {
        if (colorsSeen.find(ParityFlipped(*it)) != colorsSeen.end())
            return true;
    }
    return false;
}

/**
 * Calling a "color wing" where A+ and B+ see each other, then any cells which
 * see A- and B- cannot be true
 */
bool EliminateCandidatesThatSeeColorWing(Sudoku &sudoku,
        const ColorContainer &colors)
{
    std::set<std::pair<Color, Color> > links = BuildColorLinks(colors);
    std::vector<ColoredCandidate> changed;
    bool ret = false;
    for (std::set<std::pair<Color, Color> >::iterator i = links.begin();
            i != links.end(); ++i) {
        typedef std::set<ColoredCandidate, PositionValueComp> Coverage;
        Coverage coverageFirst = BuildColorCoverage(sudoku, colors, i->first);
        Coverage coverageSecond = BuildColorCoverage(sudoku, colors, i->second);

        for (Coverage::iterator j = coverageFirst.begin();
                j != coverageFirst.end(); ++j) {
            if (coverageSecond.find(*j) != coverageSecond.end()) {
                Cell cell = sudoku.GetCell(j->pos);
                if (cell.ExcludeCandidate(j->value)) {
                    sudoku.SetCell(cell, j->pos);
                    changed.push_back(*j);
                    ret = true;
                }
            }
        }
    }

    if (ret) {
        std::string str = ChangedCandidatesToString(changed);
        Log(Info, "3d medusa colors (candidate sees color wing) ==> %s\n",
                str.c_str());
    }

    return ret;
}

/**
 * Note that because set iterators are const (or are undefined when modified),
 * this doesn't return the actual links, but the parity flipped version. Any
 * cell which sees moth these colors must be false.
 */

std::set<std::pair<Color, Color> > BuildColorLinks(const ColorContainer &colors)
{
    std::set<std::pair<Color, Color> > ret;
    const ColorView &colorView = colors.get<1>();
    for (ColorView::iterator i = colorView.begin(); i != colorView.end(); ++i) {
        for (ColorView::iterator j = ++ColorView::iterator(i);
                j != colorView.end(); ++j) {
            if (i->color != ParityFlipped(j->color) &&
                    IsWeaklyLinked(*i, *j)) {
                ret.insert(std::make_pair(ParityFlipped(i->color),
                            ParityFlipped(j->color)));
            }
        }
    }
    return ret;
}

std::string ColorToString(const Color &color)
{
    std::ostringstream sstr;
    sstr << std::setw(2) << color.id << (color.parity ? '+' : '-');
    return sstr.str();
}

}
