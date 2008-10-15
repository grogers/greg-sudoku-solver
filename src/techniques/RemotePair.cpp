#include "Sudoku.hpp"
#include "Logging.hpp"

#include <utility>
#include <list>
#include <deque>
#include <algorithm>
#include <sstream>
#include <boost/tuple/tuple.hpp>

namespace {
typedef std::list<std::deque<Position> > RemotePairList;
RemotePairList BuildInitialRemotePairList(const Sudoku &);
boost::array<Index_t, 2> GetValuesForBivalueCell(const Cell &);
bool AddLayerToRemotePairListBack(const Sudoku &, RemotePairList &);
bool AddLayerToRemotePairListFront(const Sudoku &, RemotePairList &);
void PrintRemotePairList(const RemotePairList &);
bool CondenseRemotePairList(RemotePairList &);
bool EliminateCandidatesWithRemotePairList(Sudoku &, const RemotePairList &);
bool EliminateCandidatesWithRemotePairChain(Sudoku &,
        const std::deque<Position> &);
bool EliminateCandidatesWithRemotePairEndpoints(Sudoku &, const Position &,
        const Position &,
        std::vector<boost::tuple<Index_t, Index_t, Index_t> > &);
void LogRemotePair(const Sudoku &,
        std::deque<Position>::const_iterator,
        std::deque<Position>::const_iterator,
        const std::vector<boost::tuple<Index_t, Index_t, Index_t> > &);

}

bool RemotePair(Sudoku &sudoku)
{
    Log(Trace, "searching for remote pairs\n");
    RemotePairList remotePairList = BuildInitialRemotePairList(sudoku);
    while (true) {
        if (AddLayerToRemotePairListBack(sudoku, remotePairList)) {
            while (CondenseRemotePairList(remotePairList)) ;

            continue;
        } else if (AddLayerToRemotePairListFront(sudoku, remotePairList)) {
            while (CondenseRemotePairList(remotePairList)) ;

            continue;
        } else {
            return EliminateCandidatesWithRemotePairList(sudoku, remotePairList);
        }
    }
}

namespace {
RemotePairList BuildInitialRemotePairList(const Sudoku &sudoku)
{
    RemotePairList ret;
    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            if (sudoku.GetCell(i, j).NumCandidates() == 2) {
                ret.push_back(std::deque<Position>());
                ret.back().push_back(Position(i, j));
            }
        }
    }
    return ret;
}

boost::array<Index_t, 2> GetValuesForBivalueCell(const Cell &cell)
{
    assert(cell.NumCandidates() == 2);

    boost::array<Index_t, 2> ret;
    for (Index_t val = 1, cnt = 0; val <= 9; ++val) {
        if (cell.IsCandidate(val))
            ret[cnt++] = val;
    }
    return ret;
}

bool AddLayerToRemotePairListBack(const Sudoku &sudoku, RemotePairList &remotePairs)
{
    bool ret = false;
    for (RemotePairList::iterator i = remotePairs.begin();
            i != remotePairs.end(); ++i) {
        boost::array<Position, NUM_BUDDIES> buddies =
            sudoku.GetBuddies(i->back());
        boost::array<Index_t, 2> values =
            GetValuesForBivalueCell(sudoku.GetCell(i->back()));

        for (Index_t j = 0; j < NUM_BUDDIES; ++j) {
            if (i->back() == buddies[j])
                continue;

            Cell cell = sudoku.GetCell(buddies[j]);
            if (cell.IsCandidate(values[0]) && cell.IsCandidate(values[1]) &&
                    cell.NumCandidates() == 2 &&
                    std::find(i->begin(), i->end(), buddies[j]) == i->end()) {
                i->push_back(buddies[j]);
                ret = true;
                break;
            }
        }
    }
    return ret;
}

bool AddLayerToRemotePairListFront(const Sudoku &sudoku, RemotePairList &remotePairs)
{
    bool ret = false;
    for (RemotePairList::iterator i = remotePairs.begin();
            i != remotePairs.end(); ++i) {
        boost::array<Position, NUM_BUDDIES> buddies =
            sudoku.GetBuddies(i->front());
        boost::array<Index_t, 2> values =
            GetValuesForBivalueCell(sudoku.GetCell(i->front()));

        for (Index_t j = 0; j < NUM_BUDDIES; ++j) {
            if (i->front() == buddies[j])
                continue;

            Cell cell = sudoku.GetCell(buddies[j]);
            if (cell.IsCandidate(values[0]) && cell.IsCandidate(values[1]) &&
                    cell.NumCandidates() == 2 &&
                    std::find(i->begin(), i->end(), buddies[j]) == i->end()) {
                i->push_front(buddies[j]);
                ret = true;
                break;
            }
        }
    }
    return ret;
}

void PrintRemotePairList(const RemotePairList &remotePairs)
{
    Log(Trace, "remote pair list:\n");
    for (RemotePairList::const_iterator i = remotePairs.begin();
            i != remotePairs.end(); ++i) {
        std::ostringstream sstr;
        for (std::deque<Position>::const_iterator j = i->begin();
                j != i->end(); ++j) {
            if (j != i->begin())
                sstr << ", ";
            sstr << 'r' << j->row+1 << 'c' << j->col+1;
        }
        Log(Trace, "%s\n", sstr.str().c_str());
    }
}

bool CondenseRemotePairList(RemotePairList &remotePairs)
{
    bool ret = false;
    for (RemotePairList::iterator i = remotePairs.begin();
            i != remotePairs.end(); ++i) {
        if (i->size() == 1) {
            RemotePairList::iterator j = i--;
            remotePairs.erase(j);
            continue;
        }

        for (RemotePairList::iterator j = i; j != remotePairs.end(); ++j) {
            if (j == i)
                continue;

            if (std::find_first_of(i->begin(), i->end(),
                        j->begin(), j->end()) != i->end()) {
                remotePairs.erase(j);
                ret = true;
                break;
            }
        }
    }
    return ret;
}

bool EliminateCandidatesWithRemotePairList(Sudoku &sudoku,
        const RemotePairList &remotePairs)
{
    bool ret = false;
    for (RemotePairList::const_iterator i = remotePairs.begin();
            i != remotePairs.end(); ++i) {
        if (i->size() < 4)
            continue;

        if (EliminateCandidatesWithRemotePairChain(sudoku, *i))
            ret = true;
    }
    return ret;
}

bool EliminateCandidatesWithRemotePairChain(Sudoku &sudoku,
        const std::deque<Position> &pairChain)
{
    bool ret = false;
    for (std::deque<Position>::const_iterator i = pairChain.begin();
            i != pairChain.end(); ++i) {
        for (std::deque<Position>::const_iterator j = i + 3;
                j < pairChain.end(); j += 2) {
            std::vector<boost::tuple<Index_t, Index_t, Index_t> > changed;
            if (EliminateCandidatesWithRemotePairEndpoints(sudoku, *i, *j, changed)) {
                LogRemotePair(sudoku, i, j + 1, changed);
                ret = true;
            }
        }
    }
    return ret;
}

bool EliminateCandidatesWithRemotePairEndpoints(Sudoku &sudoku,
        const Position &x,
        const Position &y,
        std::vector<boost::tuple<Index_t, Index_t, Index_t> > &changed)
{
    bool ret = false;
    boost::array<Position, NUM_BUDDIES> buddies = sudoku.GetBuddies(x);

    for (Index_t i = 0; i < NUM_BUDDIES; ++i) {
        if (buddies[i] == x || buddies[i] == y)
            continue;

        if (IsBuddy(buddies[i], y)) {
            boost::array<Index_t, 2> values =
                GetValuesForBivalueCell(sudoku.GetCell(x));
            for (Index_t j = 0; j < 2; ++j) {
                Cell cell = sudoku.GetCell(buddies[i]);
                if (cell.ExcludeCandidate(values[j])) {
                    sudoku.SetCell(cell, buddies[i]);
                    changed.push_back(boost::make_tuple(buddies[i].row,
                                buddies[i].col, values[j]));
                    ret = true;
                }
            }
        }
    }
    return ret;

}

void LogRemotePair(const Sudoku &sudoku,
        std::deque<Position>::const_iterator begin,
        std::deque<Position>::const_iterator end,
        const std::vector<boost::tuple<Index_t, Index_t, Index_t> > &changed)
{
    std::ostringstream changedStr, chainStr;
    boost::array<Index_t, 2> values =
        GetValuesForBivalueCell(sudoku.GetCell(*begin));

    Index_t cnt = 0;
    for (std::deque<Position>::const_iterator i = begin; i != end; ++i, ++cnt) {
        if (i != begin) {
            if (cnt%2 != 0)
                chainStr << '=';
            else
                chainStr << '-';
        }
        chainStr << '(' << values[0] << values[1] << ")r" << i->row+1
            << 'c' << i->col+1;
    }
    for (Index_t j = 0; j < changed.size(); ++j) {
        if (j != 0)
            changedStr << ", ";
        changedStr << 'r' << changed[j].get<0>()+1 << 'c'
            << changed[j].get<1>()+1 << '#' << changed[j].get<2>();
    }
    Log(Info, "remote pairs %s ==> %s\n", chainStr.str().c_str(),
            changedStr.str().c_str());
}

}
