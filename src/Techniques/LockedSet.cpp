#include "Sudoku.hpp"
#include "Logging.hpp"

namespace {
bool NakedSetInHouse(House &);
std::vector<Index_t> IndicesOfPossibleNakedSet(const House &);
bool GetNewIndicesToVisit(std::vector<Index_t> &, Index_t);
bool NakedSetInHouseWithIndices(House &, const boost::array<Index_t, 4> &, Index_t);

bool HiddenSetInHouse(House &);
std::vector<Index_t> ValuesOfPossibleHiddenSet(const House &);
bool HiddenSetInHouseWithValues(House &, const boost::array<Index_t, 4> &, Index_t);
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
    Log(Trace, "searching for hidden sets\n");
    for (Index_t i = 0; i < 9; ++i) {
        House house = sudoku.GetRow(i);
        if (HiddenSetInHouse(house)) {
            sudoku.SetRow(house, i);
            return true;
        }

        house = sudoku.GetCol(i);
        if (HiddenSetInHouse(house)) {
            sudoku.SetCol(house, i);
            return true;
        }

        house = sudoku.GetBox(i);
        if (HiddenSetInHouse(house)) {
            sudoku.SetBox(house, i);
            return true;
        }
    }
    return false;
}

namespace {

std::vector<Index_t> IndicesOfPossibleNakedSet(const House &house)
{
    std::vector<Index_t> ret;
    for (Index_t i = 0; i < 9; ++i) {
        if (!house[i].HasValue())
            ret.push_back(i);
    }
    return ret;
}

std::vector<Index_t> ValuesOfPossibleHiddenSet(const House &house)
{
    std::vector<Index_t> ret;
    for (Index_t val = 1; val <= 9; ++val) {
        Index_t cnt = 0;
        for (Index_t i = 0; i < 9; ++i) {
            if (house[i].HasValue() && house[i].GetValue() == val)
                ++cnt;
        }
        if (cnt == 0)
            ret.push_back(val);
    }
    return ret;
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

bool NakedSetInHouse(House &house)
{
    std::vector<Index_t> indexList = IndicesOfPossibleNakedSet(house);
    for (Index_t order = 2; order <= indexList.size()/2; ++order) {
        std::vector<Index_t> indicesToVisit(order);
        for (Index_t i = 0; i < order; ++i)
            indicesToVisit[i] = i;

        do {
            boost::array<Index_t, 4> indexIntoHouse;
            for (Index_t i = 0; i < order; ++i)
                indexIntoHouse[i] = indexList[indicesToVisit[i]];

            if (NakedSetInHouseWithIndices(house, indexIntoHouse, order)) {
                Log(Info, "found naked set of order %d in house\n", order);
                return true;
            }
        } while (GetNewIndicesToVisit(indicesToVisit, indexList.size()));
    }
    return false;
}

/**
 * If the number of candidates in the cells of the house pointed at by
 * index are equal to order (size of index), then it is a naked set.
 */
bool NakedSetInHouseWithIndices(House &house, const boost::array<Index_t, 4> &index, Index_t order)
{
    bool ret = false;
    boost::array<Index_t, 4> candidates = {{ 0 }}; // order can't be greater than 4
    Index_t numCandidates = 0;

    for (Index_t i = 0; i < order; ++i) {
        for (Index_t val = 1; val <= 9; ++val) {
            if (house[index[i]].IsCandidate(val) &&
                    std::find(candidates.data(), candidates.data() + numCandidates, val) == candidates.data() + numCandidates) {
                if (numCandidates >= order)
                    return false;

                candidates[numCandidates++] = val;
            }
        }
    }

    if (numCandidates == order) {
        for (Index_t i = 0; i < 9; ++i) {
            if (std::find(index.data(), index.data() + order, i) != index.data() + order)
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

bool HiddenSetInHouse(House &house)
{
    std::vector<Index_t> valueList = ValuesOfPossibleHiddenSet(house);
    for (Index_t order = 2; order <= valueList.size()/2; ++order) {
        std::vector<Index_t> valuesToVisit(order);
        for (Index_t i = 0; i < order; ++i)
            valuesToVisit[i] = i;

        do {
            boost::array<Index_t, 4> valuesInSet;
            for (Index_t i = 0; i < order; ++i)
                valuesInSet[i] = valueList[valuesToVisit[i]];

            if (HiddenSetInHouseWithValues(house, valuesInSet, order)) {
                Log(Info, "found hidden set of order %d in house\n", order);
                return true;
            }
        } while (GetNewIndicesToVisit(valuesToVisit, valueList.size()));
    }
    return false;
}

/**
 *
 */
bool HiddenSetInHouseWithValues(House &house, const boost::array<Index_t, 4> &values, Index_t order)
{
    bool ret = false;
    boost::array<Index_t, 4> indices = {{ 0 }}; // order can't be greater than 4
    Index_t numIndices = 0;

    for (Index_t ival = 0; ival < order; ++ival) {
        for (Index_t i = 0; i < 9; ++i) {
            if (house[i].IsCandidate(values[ival]) &&
                    std::find(indices.data(), indices.data() + numIndices, i) == indices.data() + numIndices) {
                if (numIndices >= order)
                    return false;

                indices[numIndices++] = i;
            }
        }
    }

    if (numIndices == order) {
        for (Index_t val = 1; val <= 9; ++val) {
            if (std::find(values.data(), values.data() + order, val) != values.data() + order)
                continue;

            for (Index_t j = 0; j < order; ++j) {
                if (house[indices[j]].ExcludeCandidate(val))
                {
                    Log(Info, "found hidden set of order %d, at eliminated value %d at pos %d\n", order, val, indices[j]);
                    ret = true;
                }
            }
        }
    }
    return ret;
}

}
