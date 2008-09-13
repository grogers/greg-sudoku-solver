#include "Sudoku.hpp"
#include "Logging.hpp"

#include <utility>
#include <sstream>

namespace {
// pair is (index, value)
typedef std::vector<std::pair<Index_t, Index_t> > PairList;
bool NakedSetInHouse(House &, PairList &set, PairList &changed);
std::vector<Index_t> IndicesOfPossibleNakedSet(const House &);
bool GetNewIndicesToVisit(std::vector<Index_t> &, Index_t);
bool NakedSetInHouseWithIndices(House &, const boost::array<Index_t, 4> &, Index_t, PairList &set, PairList &changed);

bool HiddenSetInHouse(House &, PairList &set, PairList &changed);
std::vector<Index_t> ValuesOfPossibleHiddenSet(const House &);
bool HiddenSetInHouseWithValues(House &, const boost::array<Index_t, 4> &, Index_t, PairList &set, PairList &changed);

void LogChangesForRow(Index_t row, const PairList &set, const PairList &changed, const char *setType);
void LogChangesForCol(Index_t col, const PairList &set, const PairList &changed, const char *setType);
void LogChangesForBox(Index_t box, const PairList &set, const PairList &changed, const char *setType);
const char *OrderToString(Index_t);
}

bool NakedSet(Sudoku &sudoku)
{
    Log(Trace, "searching for naked sets\n");
    PairList set, changed; // used only for logging
    for (Index_t i = 0; i < 9; ++i) {
        House house = sudoku.GetRow(i);
        if (NakedSetInHouse(house, set, changed)) {
            LogChangesForRow(i, set, changed, "naked");
            sudoku.SetRow(house, i);
            return true;
        }

        house = sudoku.GetCol(i);
        if (NakedSetInHouse(house, set, changed)) {
            LogChangesForCol(i, set, changed, "naked");
            sudoku.SetCol(house, i);
            return true;
        }

        house = sudoku.GetBox(i);
        if (NakedSetInHouse(house, set, changed)) {
            LogChangesForBox(i, set, changed, "naked");
            sudoku.SetBox(house, i);
            return true;
        }
    }
    return false;
}

bool HiddenSet(Sudoku &sudoku)
{
    Log(Trace, "searching for hidden sets\n");
    PairList set, changed; // used only for logging
    for (Index_t i = 0; i < 9; ++i) {
        House house = sudoku.GetRow(i);
        if (HiddenSetInHouse(house, set, changed)) {
            LogChangesForRow(i, set, changed, "hidden");
            sudoku.SetRow(house, i);
            return true;
        }

        house = sudoku.GetCol(i);
        if (HiddenSetInHouse(house, set, changed)) {
            LogChangesForCol(i, set, changed, "hidden");
            sudoku.SetCol(house, i);
            return true;
        }

        house = sudoku.GetBox(i);
        if (HiddenSetInHouse(house, set, changed)) {
            LogChangesForBox(i, set, changed, "hidden");
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

bool NakedSetInHouse(House &house, PairList &set, PairList &changed)
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

            if (NakedSetInHouseWithIndices(house, indexIntoHouse, order, set, changed))
                return true;
        } while (GetNewIndicesToVisit(indicesToVisit, indexList.size()));
    }
    return false;
}

/**
 * If the number of candidates in the cells of the house pointed at by
 * index are equal to order (size of index), then it is a naked set.
 */
bool NakedSetInHouseWithIndices(House &house, const boost::array<Index_t, 4> &index, Index_t order, PairList &set, PairList &changed)
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
        changed.clear();
        for (Index_t i = 0; i < 9; ++i) {
            if (std::find(index.data(), index.data() + order, i) != index.data() + order)
                continue;

            for (Index_t j = 0; j < order; ++j) {
                if (house[i].ExcludeCandidate(candidates[j]))
                {
                    changed.push_back(std::make_pair(i, candidates[j]));
                    ret = true;
                }
            }
        }
    }

    if (ret) {
        set.resize(order);
        for (Index_t i = 0; i < order; ++i) {
            set[i].first = index[i];
            set[i].second = candidates[i];
        }
    }

    return ret;
}

bool HiddenSetInHouse(House &house, PairList &set, PairList &changed)
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

            if (HiddenSetInHouseWithValues(house, valuesInSet, order, set, changed))
                return true;
        } while (GetNewIndicesToVisit(valuesToVisit, valueList.size()));
    }
    return false;
}

/**
 *
 */
bool HiddenSetInHouseWithValues(House &house, const boost::array<Index_t, 4> &values, Index_t order, PairList &set, PairList &changed)
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
        changed.clear();
        for (Index_t val = 1; val <= 9; ++val) {
            if (std::find(values.data(), values.data() + order, val) != values.data() + order)
                continue;

            for (Index_t j = 0; j < order; ++j) {
                if (house[indices[j]].ExcludeCandidate(val))
                {
                    changed.push_back(std::make_pair(indices[j], val));
                    ret = true;
                }
            }
        }
    }

    if (ret) {
        set.resize(order);
        for (Index_t i = 0; i < order; ++i) {
            set[i].first = indices[i];
            set[i].second = values[i];
        }
    }

    return ret;
}

void LogChangesForRow(Index_t row, const PairList &set, const PairList &changed, const char *setType)
{
    Index_t order = set.size();
    std::ostringstream colsInSet, valsInSet, changedStr;

    for (Index_t i = 0; i < order; ++i) {
        colsInSet << set[i].first+1;
        valsInSet << set[i].second;
    }

    for (Index_t i = 0; i < changed.size(); ++i) {
        if (i != 0)
            changedStr << ", ";
        changedStr << 'r' << row+1 << 'c' << changed[i].first+1 << '#'
            << changed[i].second;
    }

    Log(Info, "%s %s r%dc%s=%s ==> %s\n", setType, OrderToString(order),
            row+1, colsInSet.str().c_str(), valsInSet.str().c_str(),
            changedStr.str().c_str());
}

void LogChangesForCol(Index_t col, const PairList &set, const PairList &changed, const char *setType)
{
    Index_t order = set.size();
    std::ostringstream rowsInSet, valsInSet, changedStr;

    for (Index_t i = 0; i < order; ++i) {
        rowsInSet << set[i].first+1;
        valsInSet << set[i].second;
    }

    for (Index_t i = 0; i < changed.size(); ++i) {
        if (i != 0)
            changedStr << ", ";
        changedStr << 'r' << changed[i].first+1 << 'c' << col+1 << '#'
            << changed[i].second;
    }

    Log(Info, "%s %s r%sc%d=%s ==> %s\n", setType, OrderToString(order),
            rowsInSet.str().c_str(), col+1, valsInSet.str().c_str(),
            changedStr.str().c_str());
}

void LogChangesForBox(Index_t box, const PairList &set, const PairList &changed, const char *setType)
{
    Index_t order = set.size();
    std::ostringstream setStr, valsInSet, changedStr;

    for (Index_t i = 0; i < order; ++i) {
        valsInSet << set[i].second;
    }

    for (Index_t i = 0, lastRow = 10; i < order; ++i) {
        Index_t row = RowForCellInBox(box, set[i].first);
        Index_t col = ColForCellInBox(box, set[i].first);

        if (row == lastRow) {
            setStr << col+1;
        } else {
            if (i != 0)
                setStr << '&';
            setStr << 'r' << row+1 << 'c' << col+1;
            lastRow = row;
        }
    }

    for (Index_t i = 0; i < changed.size(); ++i) {
        if (i != 0)
            changedStr << ", ";

        changedStr << 'r' << RowForCellInBox(box, changed[i].first)+1
            << 'c' << ColForCellInBox(box, changed[i].first)+1
            << '#' << changed[i].second;
    }

    Log(Info, "%s %s %s=%s ==> %s\n", setType, OrderToString(order),
            setStr.str().c_str(), valsInSet.str().c_str(),
            changedStr.str().c_str());
}

const char *OrderToString(Index_t order)
{
    switch (order) {
        case 1: return "single";
        case 2: return "pair";
        case 3: return "triplet";
        case 4: return "quad";
        default: return "unknown";
    }
}


}
