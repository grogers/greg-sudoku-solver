#include "Sudoku.hpp"
#include "Logging.hpp"
#include "Coloring.hpp"

#include <climits>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <functional>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/key_extractors.hpp>

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
    ColoredCandidate(Position pos, Index_t value, Color color) :
        pos(pos), value(value), color(color) {}
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

}


bool MedusaColor(Sudoku &sudoku)
{
    Log(Trace, "searching for 3d medusa color eliminations\n");

    ColorContainer colors = BuildMedusaColors(sudoku);

    return false;
}


namespace {

ColorContainer BuildMedusaColors(const Sudoku &sudoku)
{
    ColorContainer ret;

    return ret;
}

}
