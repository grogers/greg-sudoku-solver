#include "Sudoku.hpp"
#include "Logging.hpp"

namespace {
bool NakedSetInHouse(House &);
std::vector<Index_t> IndicesOfPossibleLockedSet(const House &);
bool GetNewIndicesToVisit(std::vector<Index_t> &, Index_t);
bool NakedSetInHouseWithIndices(House &, const std::vector<Index_t> &);

}

bool NakedSet(Sudoku &sudoku)
{
    Log(Trace, "searching for naked sets\n");
    for (Index_t i = 0; i < 9; ++i) {
        House house = sudoku.GetRow(i);
        if (NakedSetInHouse(house)) {
            sudoku.SetRow(house, i);
            return true;
        }

        house = sudoku.GetCol(i);
        if (NakedSetInHouse(house)) {
            sudoku.SetCol(house, i);
            return true;
        }

        house = sudoku.GetBox(i);
        if (NakedSetInHouse(house)) {
            sudoku.SetBox(house, i);
            return true;
        }
    }
    return false;
}

bool HiddenSet(Sudoku &sudoku)
{
    /// @fixme
    return false;
}

namespace {

std::vector<Index_t> IndicesOfPossibleLockedSet(const House &house)
{
    std::vector<Index_t> ret;
    for (Index_t i = 0; i < 9; ++i) {
        if (!house[i].HasValue())
            ret.push_back(i);
    }
    return ret;
}

bool NakedSetInHouse(House &house)
{
    std::vector<Index_t> indexList = IndicesOfPossibleLockedSet(house);

    for (Index_t order = 2; order <= indexList.size()/2; ++order) {
        std::vector<Index_t> indicesToVisit(order);
        for (Index_t i = 0; i < order; ++i)
            indicesToVisit[i] = i;

        do {
            std::vector<Index_t> indexIntoHouse(order);
            for (Index_t i = 0; i < order; ++i)
                indexIntoHouse[i] = indexList[indicesToVisit[i]];

            if (NakedSetInHouseWithIndices(house, indexIntoHouse)) {
                Log(Info, "found naked set of order %d in house\n", order);
                return true;
            }
        } while (GetNewIndicesToVisit(indicesToVisit, indexList.size()));
    }
    return false;
}

bool GetNewIndicesToVisit(std::vector<Index_t> &indicesToVisit, Index_t n)
{
    const Index_t order = indicesToVisit.size();
    for (Index_t curr = order - 1; ; --curr) {
        if (indicesToVisit[curr] < n - order + curr) {
            ++indicesToVisit[curr];
            return true;
        }

        if (curr == 0)
            return false;

        if (indicesToVisit[curr] > indicesToVisit[curr - 1] + 1) {
            indicesToVisit[curr] = indicesToVisit[curr - 1] + 2;
            ++indicesToVisit[curr - 1];
            return true;
        }
    }
}

/**
 * If the number of candidates in the cells of the house pointed at by
 * index are equal to order (size of index), then it is a naked set.
 */
bool NakedSetInHouseWithIndices(House &house, const std::vector<Index_t> &index)
{
    const Index_t order = index.size();
    bool ret = false;
    boost::array<Index_t, 4> candidates = {{ 0 }}; // order can't be greater than 4
    Index_t numCandidates = 0;

    for (Index_t i = 0; i < order; ++i) {
        for (Index_t val = 1; val <= 9; ++val) {
            if (house[index[i]].IsCandidate(val) &&
                    std::find(candidates.begin(), candidates.end(), val) == candidates.end()) {
                if (numCandidates >= 4)
                    return false;

                candidates[numCandidates++] = val;
            }
        }
    }

    if (numCandidates == order) {
        for (Index_t i = 0; i < 9; ++i) {
            if (std::find(index.begin(), index.end(), i) != index.end())
                continue;

            for (Index_t j = 0; j < order; ++j) {
                if (house[i].ExcludeCandidate(candidates[j]))
                {
                    Log(Info, "found naked set of order %d, at eliminated value %d at pos %d\n", order, candidates[j], i);
                    ret = true;
                }
            }
        }
    }
    return ret;
}

}
