#ifndef COLORING_HPP
#define COLORING_HPP

#include "DefineGroup.hpp"

DEFINE_PAIR(Color, Index_t, bool, id, parity);

inline
Color ParityFlipped(const Color &x)
{
    return Color(x.id, !x.parity);
}



#endif
