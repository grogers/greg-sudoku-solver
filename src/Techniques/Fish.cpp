#include "Sudoku.hpp"
#include "Logging.hpp"
#include "LockedSet.hpp"

#include <sstream>
#include <set>
#include <iostream>

namespace {
const char *FishSize(Index_t);
bool BasicFishForValue(Sudoku &, Index_t value);
bool FrankenFishForValue(Sudoku &, Index_t value);
bool MutantFishForValue(Sudoku &, Index_t value);
enum HouseType { ROW, COL, BOX };
enum FishShape { BASIC, FRANKEN, MUTANT };
bool SectorsNotBasicFish(const boost::array<std::pair<HouseType, Index_t>, 6> &,
        const boost::array<std::pair<HouseType, Index_t>, 6> &, Index_t);
bool SectorNotBasicFish(const boost::array<std::pair<HouseType, Index_t>, 6> &, Index_t);
bool FishWithIndices(Sudoku &,
        const boost::array<std::pair<HouseType, Index_t>, 6> &,
        const boost::array<std::pair<HouseType, Index_t>, 6> &,
        Index_t, Index_t, FishShape);
bool SectorsNotFrankenFish(const boost::array<std::pair<HouseType, Index_t>, 6> &,
        const boost::array<std::pair<HouseType, Index_t>, 6> &, Index_t);
bool SectorNotFrankenFish(const boost::array<std::pair<HouseType, Index_t>, 6> &, Index_t);
bool SectorsNotMutantFish(const boost::array<std::pair<HouseType, Index_t>, 6> &,
        const boost::array<std::pair<HouseType, Index_t>, 6> &, Index_t);
bool FrankenFishWithIndices(Sudoku &,
        const boost::array<std::pair<HouseType, Index_t>, 6> &,
        const boost::array<std::pair<HouseType, Index_t>, 6> &,
        Index_t, Index_t);
std::vector<std::pair<HouseType, Index_t> > PossibleRowColSectors(const Sudoku &, Index_t);
std::vector<std::pair<HouseType, Index_t> > AllPossibleSectors(const Sudoku &, Index_t);
bool IsHouseOpenOnValue(const House &house, Index_t value);
bool IsCellInHouse(Index_t, Index_t, HouseType, Index_t);
int Vertixness(Index_t row, Index_t col,
        const boost::array<std::pair<HouseType, Index_t>, 6> &base,
        const boost::array<std::pair<HouseType, Index_t>, 6> &cover, Index_t order);
std::set<std::pair<Index_t, Index_t> > BuildListOfFins(const Sudoku &,
        const boost::array<std::pair<HouseType, Index_t>, 6> &base,
        const boost::array<std::pair<HouseType, Index_t>, 6> &cover,
        Index_t value, Index_t order);
bool MakeEliminations(Sudoku &sudoku,
        const boost::array<std::pair<HouseType, Index_t>, 6> &base,
        const boost::array<std::pair<HouseType, Index_t>, 6> &cover,
        Index_t value, Index_t order,
        const std::set<std::pair<Index_t, Index_t> > &fins, FishShape);
void LogChanges(const std::vector<std::pair<Index_t, Index_t> > &,
        const boost::array<std::pair<HouseType, Index_t>, 6> &base,
        const boost::array<std::pair<HouseType, Index_t>, 6> &cover,
        Index_t value, Index_t order,
        const std::set<std::pair<Index_t, Index_t> > &fins, FishShape shape);
void OutputSectors(std::ostringstream &,
        const boost::array<std::pair<HouseType, Index_t>, 6> &, Index_t);
bool CellSeesAllFins(Index_t row, Index_t col,
        const std::set<std::pair<Index_t, Index_t> > &fins);

}

bool FinnedFish(Sudoku &sudoku)
{
    Log(Trace, "searching for finned fish\n");
    for (Index_t val = 1; val <= 9; ++val) {
        if (BasicFishForValue(sudoku, val))
            return true;
    }
    return false;
}

bool FrankenFish(Sudoku &sudoku)
{
    Log(Trace, "searching for franken fish\n");
    for (Index_t val = 1; val <= 9; ++val) {
        if (FrankenFishForValue(sudoku, val))
            return true;
    }
    return false;
}

bool MutantFish(Sudoku &sudoku)
{
    Log(Trace, "searching for mutant fish\n");
    for (Index_t val = 1; val <= 9; ++val) {
        if (MutantFishForValue(sudoku, val))
            return true;
    }
    return false;
}

namespace {

bool BasicFishForValue(Sudoku &sudoku, Index_t value)
{
    const std::vector<std::pair<HouseType, Index_t> > possSectors =
        PossibleRowColSectors(sudoku, value);

    // find all exemplars (x-wing to whale)
    Index_t max = possSectors.size()/2;
    if (max >= 1)
        --max;
    max = std::min<Index_t>(max, 6);
    for (Index_t order = 2; order <= max; ++order) {
        std::vector<Index_t> baseSectorIndex(order);
        for (Index_t i = 0; i < order; ++i)
            baseSectorIndex[i] = i;

        do {
            boost::array<std::pair<HouseType, Index_t>, 6> base;
            for (Index_t i = 0; i < order; ++i)
                 base[i] = possSectors[baseSectorIndex[i]];

            if (SectorNotBasicFish(base, order))
                continue;

            std::vector<Index_t> coverSectorIndex(order);
            for (Index_t i = 0; i < order; ++i)
                coverSectorIndex[i] = i;

            do {
                boost::array<std::pair<HouseType, Index_t>, 6> cover;
                for (Index_t i = 0; i < order; ++i)
                     cover[i] = possSectors[coverSectorIndex[i]];

                if (SectorNotBasicFish(cover, order))
                    continue;

                if (SectorsNotBasicFish(base, cover, order))
                    continue;

                if (FishWithIndices(sudoku, base, cover, value, order, BASIC))
                    return true;
            } while (GetNewIndicesToVisit(coverSectorIndex, possSectors.size()));
        } while (GetNewIndicesToVisit(baseSectorIndex, possSectors.size()));
    }

    return false;
}

bool FrankenFishForValue(Sudoku &sudoku, Index_t value)
{
    const std::vector<std::pair<HouseType, Index_t> > possSectors =
        AllPossibleSectors(sudoku, value);

    // find all exemplars (x-wing to whale)
    Index_t max = possSectors.size()/3;
    if (max >= 1)
        --max;
    max = std::min<Index_t>(max, 6);
    for (Index_t order = 2; order <= max; ++order) {
        std::vector<Index_t> baseSectorIndex(order);
        for (Index_t i = 0; i < order; ++i)
            baseSectorIndex[i] = i;

        do {
            boost::array<std::pair<HouseType, Index_t>, 6> base;
            for (Index_t i = 0; i < order; ++i)
                 base[i] = possSectors[baseSectorIndex[i]];

            if (SectorNotFrankenFish(base, order))
                continue;

            std::vector<Index_t> coverSectorIndex(order);
            for (Index_t i = 0; i < order; ++i)
                coverSectorIndex[i] = i;

            do {
                boost::array<std::pair<HouseType, Index_t>, 6> cover;
                for (Index_t i = 0; i < order; ++i)
                     cover[i] = possSectors[coverSectorIndex[i]];

                if (SectorNotFrankenFish(cover, order))
                    continue;

                if (SectorsNotFrankenFish(base, cover, order))
                    continue;

                if (FishWithIndices(sudoku, base, cover, value, order, FRANKEN))
                    return true;
            } while (GetNewIndicesToVisit(coverSectorIndex, possSectors.size()));
        } while (GetNewIndicesToVisit(baseSectorIndex, possSectors.size()));
    }

    return false;
}

bool MutantFishForValue(Sudoku &sudoku, Index_t value)
{
    const std::vector<std::pair<HouseType, Index_t> > possSectors =
        AllPossibleSectors(sudoku, value);

    // find all exemplars (x-wing to whale)
    Index_t max = possSectors.size()/3;
    if (max >= 1)
        --max;
    max = std::min<Index_t>(max, 6);
    for (Index_t order = 2; order <= max; ++order) {
        std::vector<Index_t> baseSectorIndex(order);
        for (Index_t i = 0; i < order; ++i)
            baseSectorIndex[i] = i;

        do {
            boost::array<std::pair<HouseType, Index_t>, 6> base;
            for (Index_t i = 0; i < order; ++i)
                 base[i] = possSectors[baseSectorIndex[i]];

            std::vector<Index_t> coverSectorIndex(order);
            for (Index_t i = 0; i < order; ++i)
                coverSectorIndex[i] = i;

            do {
                boost::array<std::pair<HouseType, Index_t>, 6> cover;
                for (Index_t i = 0; i < order; ++i)
                     cover[i] = possSectors[coverSectorIndex[i]];

                if (SectorsNotMutantFish(base, cover, order))
                    continue;

                if (FishWithIndices(sudoku, base, cover, value, order, MUTANT))
                    return true;
            } while (GetNewIndicesToVisit(coverSectorIndex, possSectors.size()));
        } while (GetNewIndicesToVisit(baseSectorIndex, possSectors.size()));
    }

    return false;
}

bool SectorNotBasicFish(const boost::array<std::pair<HouseType, Index_t>, 6> &sector, Index_t order)
{
    for (Index_t i = 1; i < order; ++i) {
        if (sector[i].first != sector[0].first)
            return true;
    }
    return false;
}

bool SectorsNotBasicFish(const boost::array<std::pair<HouseType, Index_t>, 6> &base,
        const boost::array<std::pair<HouseType, Index_t>, 6> &cover, Index_t order)
{
    // base must be different than the cover, ie one is rows, one is columns
    if (base[0].first == cover[0].first)
        return true;
    else
        return false;
}

bool SectorNotFrankenFish(const boost::array<std::pair<HouseType, Index_t>, 6> &sector, Index_t order)
{
    // franken fish are row + boxes by cols + boxes or vv.
    HouseType lineType = static_cast<HouseType>(-1);
    bool found = false;
    for (Index_t i = 0; i < order; ++i) {
        if (found && lineType != sector[i].first)
            return true;

        if (!found) {
            if (sector[i].first == ROW || sector[i].first == COL) {
                    found = true;
                    lineType = sector[i].first;
            }
        }
    }
    return false;
}

bool SectorsNotFrankenFish(const boost::array<std::pair<HouseType, Index_t>, 6> &base,
        const boost::array<std::pair<HouseType, Index_t>, 6> &cover, Index_t order)
{
    // franken fish are row + boxes by cols + boxes or vv.
    HouseType lineType = static_cast<HouseType>(-1);
    bool found = false;
    for (Index_t i = 0; i < order; ++i) {
        if (!found) {
            if (base[i].first == ROW || base[i].first == COL) {
                    found = true;
                    lineType = base[i].first;
            }
        }
    }

    for (Index_t i = 0; i < order; ++i) {
        if (found && lineType == cover[i].first)
            return true;
    }

    // box indexes can't be the same
    for (Index_t i = 0; i < order; ++i) {
        if (base[i].first != BOX)
            continue;

        for (Index_t j = 0; j < order; ++j) {
            if (cover[j].first != BOX)
                continue;

            if (base[i].second == cover[j].second)
                return true;
        }
    }
    return false;
}

bool SectorsNotMutantFish(const boost::array<std::pair<HouseType, Index_t>, 6> & base,
        const boost::array<std::pair<HouseType, Index_t>, 6> &cover, Index_t order)
{
    for (Index_t i = 0; i < order; ++i) {
        for (Index_t j = 0; j < order; ++j) {
            if (base[i].first == cover[j].first &&
                    base[i].second == cover[j].second) {
                return true;
            }
        }
    }
    return false;
}

std::vector<std::pair<HouseType, Index_t> >
PossibleRowColSectors(const Sudoku &sudoku, Index_t value)
{
    std::vector<std::pair<HouseType, Index_t> > ret;

    for (Index_t i = 0; i < 9; ++i) {
        if (IsHouseOpenOnValue(sudoku.GetRow(i), value))
            ret.push_back(std::make_pair(ROW, i));
    }

    for (Index_t i = 0; i < 9; ++i) {
        if (IsHouseOpenOnValue(sudoku.GetCol(i), value))
            ret.push_back(std::make_pair(COL, i));
    }
    assert(ret.size()%2 == 0); // i think this must be the case
    return ret;
}

std::vector<std::pair<HouseType, Index_t> >
AllPossibleSectors(const Sudoku &sudoku, Index_t value)
{
    std::vector<std::pair<HouseType, Index_t> > ret =
        PossibleRowColSectors(sudoku, value);

    for (Index_t i = 0; i < 9; ++i) {
        if (IsHouseOpenOnValue(sudoku.GetBox(i), value))
            ret.push_back(std::make_pair(BOX, i));
    }
    assert(ret.size()%3 == 0); // i think this must be the case
    return ret;
}

bool IsHouseOpenOnValue(const House &house, Index_t value)
{
    for (Index_t i = 0; i < 9; ++i) {
        if (house[i].HasValue() && house[i].GetValue() == value)
            return false;
    }
    return true;
}

bool IsCellInHouse(Index_t row, Index_t col, HouseType type, Index_t idx)
{
    switch (type) {
        case ROW: return row == idx;
        case COL: return col == idx;
        case BOX: return BoxIndex(row, col) == idx;
        default: return false;
    }
}

/**
 * @return number of base sectors minus the number of cover sectors a cell is
 * contained in.
 *
 * If this value is negative, any candidate there is an elimination if it sees
 * all fins (or there are no fins)
 * If this value is positive, any candidate there is a fin.
 * If this value is zero, a candidate may exist there but doesn't have to.
 */
int Vertixness(Index_t row, Index_t col,
        const boost::array<std::pair<HouseType, Index_t>, 6> &base,
        const boost::array<std::pair<HouseType, Index_t>, 6> &cover, Index_t order)
{
    int ret = 0;
    for (Index_t i = 0; i < order; ++i) {
        ret += IsCellInHouse(row, col, base[i].first, base[i].second);
        ret -= IsCellInHouse(row, col, cover[i].first, cover[i].second);
    }
    return ret;
}

std::set<std::pair<Index_t, Index_t> > BuildListOfFins(const Sudoku &sudoku,
        const boost::array<std::pair<HouseType, Index_t>, 6> &base,
        const boost::array<std::pair<HouseType, Index_t>, 6> &cover,
        Index_t value, Index_t order)
{
    std::set<std::pair<Index_t, Index_t> > ret;
    for (Index_t i = 0; i < order; ++i) {
        switch (base[i].first) {
            case ROW:
                for (Index_t j = 0; j < 9; ++j) {
                    Index_t row = base[i].second, col = j;
                    Cell cell = sudoku.GetCell(row, col);
                    if (cell.IsCandidate(value) &&
                            Vertixness(row, col, base, cover, order) > 0)
                        ret.insert(std::make_pair(row, col));
                }
                break;
            case COL:
                for (Index_t j = 0; j < 9; ++j) {
                    Index_t row = j, col = base[i].second;
                    Cell cell = sudoku.GetCell(row, col);
                    if (cell.IsCandidate(value) &&
                            Vertixness(row, col, base, cover, order) > 0)
                        ret.insert(std::make_pair(row, col));
                }
                break;
            case BOX:
                for (Index_t j = 0; j < 9; ++j) {
                    Index_t row = RowForCellInBox(base[i].second, j);
                    Index_t col = ColForCellInBox(base[i].second, j);
                    Cell cell = sudoku.GetCell(row, col);
                    if (cell.IsCandidate(value) &&
                            Vertixness(row, col, base, cover, order) > 0)
                        ret.insert(std::make_pair(row, col));
                }
                break;
        }
    }
    return ret;
}

bool FishWithIndices(Sudoku &sudoku,
        const boost::array<std::pair<HouseType, Index_t>, 6> &base,
        const boost::array<std::pair<HouseType, Index_t>, 6> &cover,
        Index_t value, Index_t order, FishShape shape)
{
    std::set<std::pair<Index_t, Index_t> > fins =
        BuildListOfFins(sudoku, base, cover, value, order);
    return MakeEliminations(sudoku, base, cover, value, order, fins, shape);
}

bool MakeEliminations(Sudoku &sudoku,
        const boost::array<std::pair<HouseType, Index_t>, 6> &base,
        const boost::array<std::pair<HouseType, Index_t>, 6> &cover,
        Index_t value, Index_t order, const std::set<std::pair<Index_t, Index_t> > &fins,
        FishShape shape)
{
    bool ret = false;
    std::vector<std::pair<Index_t, Index_t> > changed;
    for (Index_t i = 0; i < order; ++i) {
        switch (cover[i].first) {
            case ROW:
                for (Index_t j = 0; j < 9; ++j) {
                    Index_t row = cover[i].second, col = j;
                    if (Vertixness(row, col, base, cover, order) < 0 &&
                            CellSeesAllFins(row, col, fins)) {
                        Cell cell = sudoku.GetCell(row, col);
                        if (cell.ExcludeCandidate(value)) {
                            sudoku.SetCell(cell, row, col);
                            changed.push_back(std::make_pair(row, col));
                            ret = true;
                        }
                    }
                }
                break;
            case COL:
                for (Index_t j = 0; j < 9; ++j) {
                    Index_t row = j, col = cover[i].second;
                    if (Vertixness(row, col, base, cover, order) < 0 &&
                            CellSeesAllFins(row, col, fins)) {
                        Cell cell = sudoku.GetCell(row, col);
                        if (cell.ExcludeCandidate(value)) {
                            sudoku.SetCell(cell, row, col);
                            changed.push_back(std::make_pair(row, col));
                            ret = true;
                        }
                    }
                }
                break;
            case BOX:
                for (Index_t j = 0; j < 9; ++j) {
                    Index_t row = RowForCellInBox(cover[i].second, j);
                    Index_t col = ColForCellInBox(cover[i].second, j);
                    if (Vertixness(row, col, base, cover, order) < 0 &&
                            CellSeesAllFins(row, col, fins)) {
                        Cell cell = sudoku.GetCell(row, col);
                        if (cell.ExcludeCandidate(value)) {
                            sudoku.SetCell(cell, row, col);
                            changed.push_back(std::make_pair(row, col));
                            ret = true;
                        }
                    }
                }
                break;
        }
    }

    if (ret)
        LogChanges(changed, base, cover, value, order, fins, shape);

    return ret;
}

void LogChanges(const std::vector<std::pair<Index_t, Index_t> > &changed,
        const boost::array<std::pair<HouseType, Index_t>, 6> &base,
        const boost::array<std::pair<HouseType, Index_t>, 6> &cover,
        Index_t value, Index_t order,
        const std::set<std::pair<Index_t, Index_t> > &fins, FishShape shape)
{
    std::ostringstream shapeStr, fishStr, changedStr;

    if (!fins.empty())
        shapeStr << "finned ";

    switch (shape) {
        case BASIC:
            if (fins.empty())
                shapeStr << "basic ";
            break;
        case FRANKEN:
            shapeStr << "franken ";
            break;
        case MUTANT:
            shapeStr << "mutant ";
            break;
    }
    shapeStr << FishSize(order);

    OutputSectors(fishStr, base, order);
    fishStr << '\\';
    OutputSectors(fishStr, cover, order);
    if (!fins.empty())
        fishStr << "+{";
    for (std::set<std::pair<Index_t, Index_t> >::const_iterator i = fins.begin();
            i != fins.end(); ++i) {
        if (i != fins.begin())
            fishStr << ',';
        fishStr << 'r' << i->first+1 << 'c' << i->second+1;
    }
    if (!fins.empty())
        fishStr << '}';
    fishStr << '=' << value;

    for (std::vector<std::pair<Index_t, Index_t> >::const_iterator i = changed.begin();
            i != changed.end(); ++i) {
        if (i != changed.begin())
            changedStr << ", ";
        changedStr << 'r' << i->first+1 << 'c' << i->second+1 << '#'
            << value;
    }

    Log(Info, "%s %s ==> %s\n", shapeStr.str().c_str(), fishStr.str().c_str(),
            changedStr.str().c_str());
}

void OutputSectors(std::ostringstream &sstr,
        const boost::array<std::pair<HouseType, Index_t>, 6> &sectors,
        Index_t order)
{
    HouseType curr = static_cast<HouseType>(-1);
    for (Index_t i = 0; i < order; ++i) {
        if (sectors[i].first == ROW) {
            if (curr != ROW)
                sstr << 'r';
            curr = ROW;
            sstr << sectors[i].second+1;
        } else if (sectors[i].first == COL) {
            if (curr != COL)
                sstr << 'c';
            curr = COL;
            sstr << sectors[i].second+1;
        } else if (sectors[i].first == BOX) {
            if (curr != BOX)
                sstr << 'b';
            curr = BOX;
            sstr << sectors[i].second+1;
        }
    }
}

bool CellSeesAllFins(Index_t row, Index_t col,
        const std::set<std::pair<Index_t, Index_t> > &fins)
{
    for (std::set<std::pair<Index_t, Index_t> >::const_iterator i = fins.begin();
            i != fins.end(); ++i) {
        if (!IsBuddy(row, col, i->first, i->second))
            return false;
    }
    return true;
}

const char *FishSize(Index_t order)
{
    switch (order) {
        case 1: return "1-fish";
        case 2: return "x-wing";
        case 3: return "swordfish";
        case 4: return "jellyfish";
        case 5: return "squirmbag";
        case 6: return "whale";
        case 7: return "leviathan";
        default: return "unknown";
    }
}

}
