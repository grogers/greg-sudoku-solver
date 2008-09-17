#include "Cell.hpp"

std::vector<Index_t> CandidatesForCell(const Cell &cell)
{
    std::vector<Index_t> ret;
    ret.reserve(cell.NumCandidates());
    for (Index_t val = 1; val <= 9; ++val) {
        if (cell.IsCandidate(val))
            ret.push_back(val);
    }
    return ret;
}

