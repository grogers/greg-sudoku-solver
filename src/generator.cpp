#include "Sudoku.hpp"
#include "Logging.hpp"
#include "Techniques.hpp"

#include <list>
#include <vector>
#include <string>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <climits>
#include <boost/tokenizer.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/random.hpp>


using namespace std;

namespace {

    struct GeneratorOptions {
        uint32_t seed;
        uint32_t numToGenerate;

        GeneratorOptions() : seed(0), numToGenerate(1) {}
    };

    const std::vector<Technique> techniques =
        boost::assign::list_of(&NakedSingle)(&HiddenSingle);

    void ConvertCmdline(list<string> &, int argc, char **argv);
    void ParseOptions(const list<string> &, GeneratorOptions &);
    void usage();
    Index_t Random(Index_t, Index_t, boost::mt19937 &);
    Sudoku GenerateSudoku(boost::mt19937 &);
    Sudoku GenerateFilledSudoku(boost::mt19937 &);
    void PruneExtraCellsFromSudoku(Sudoku &, boost::mt19937 &);
    void CrossHatchAll(Sudoku &);
}

int main(int argc, char **argv)
{
    list<string> cmdline;
    ConvertCmdline(cmdline, argc, argv);
    GeneratorOptions opts;
    ParseOptions(cmdline, opts);
    SetLogLevel(Never);

    boost::mt19937 random_state(opts.seed);

    for (uint32_t i = 0; i < opts.numToGenerate; ++i) {
        Sudoku sudoku = GenerateSudoku(random_state);
        sudoku.Output(cout, Sudoku::SingleLine);
    }

    return 0;
}

namespace {

void ConvertCmdline(list<string> &out, int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
        out.push_back(argv[i]);
}

void ParseOptions(const list<string> &cmdline, GeneratorOptions &opts)
{
    for (list<string>::const_iterator i = cmdline.begin(); i != cmdline.end(); ++i) {
        if (*i == "--help" || *i == "-h") {
            usage();
        } else if (*i == "--random-seed" || *i == "-s") {
            if (++i == cmdline.end()) {
                Log(Fatal, "No argument given to option --seed\n");
                exit(1);
            }

            std::istringstream sstr(*i);
            sstr >> opts.seed;
        } else if (*i == "--puzzles-to-generate" || *i == "-n") {
            if (++i == cmdline.end()) {
                Log(Fatal, "No argument given to option --puzzles-to-generate\n");
                exit(1);
            }

            std::istringstream sstr(*i);
            sstr >> opts.numToGenerate;
        } else {
            Log(Fatal, "Invalid argument \'%s\' specified\n", i->c_str());
            usage();
        }
    }
}

void usage()
{
    cout << "usage: generator [options]\n"
       "options:\n"
       "    --help, -h                  Print this help message.\n\n"
       "    --random-seed, -s           Seed for the random number generator, default: 0\n"
       "    --puzzles-to-generate, -n   Number of puzzles to generate\n"
       "\n"
       "return value:\n"
       "    0 - all puzzles were generated\n"
       "    1 - an error occurred specifying command line arguments\n"

       ;

    exit(0);
}

Index_t Random(Index_t min, Index_t max, boost::mt19937 &state)
{
    boost::uniform_int<> rng(min, max);
    return rng(state);
}

Sudoku GenerateSudoku(boost::mt19937 &state)
{
    Sudoku ret = GenerateFilledSudoku(state);
    PruneExtraCellsFromSudoku(ret, state);
    return ret;
}

Sudoku GenerateFilledSudoku(boost::mt19937 &state)
{
    Sudoku ret;

    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            std::vector<Index_t> candidates =
                CandidatesForCell(ret.GetCell(i, j));
            Index_t rand = Random(0, candidates.size() - 1, state);
            Sudoku tmp(ret);
            Cell cell(candidates[rand]);
            tmp.SetCell(cell, i, j);
            tmp.CrossHatch(i, j);

            // if added cell keeps puzzle unique
            if (tmp.Solve(techniques, true) != 0) {
                ret.SetCell(cell, i, j);
                ret.CrossHatch(i, j);
            } else {
                // fall back to trying to add all the digits until one is found
                // that keeps the puzzle unique
                for (Index_t k = 0; k < candidates.size(); ++k) {
                    Sudoku s(ret);
                    Cell c(candidates[k]);
                    s.SetCell(c, i, j);
                    s.CrossHatch(i, j);
                    if (s.Solve(techniques, true) != 0) {
                        ret.SetCell(c, i, j);
                        ret.CrossHatch(i, j);
                        break;
                    }
                }
            }
            assert(ret.GetCell(i, j).HasValue());
        }
    }

    return ret;
}

void PruneExtraCellsFromSudoku(Sudoku &sudoku, boost::mt19937 &state)
{
    bool changed;
    do {
        changed = false;

        for (Index_t i = 0; i < 9; ++i) {
            for (Index_t j = 0; j < 9; ++j) {
                if (!sudoku.GetCell(i, j).HasValue())
                    continue;

                Sudoku tmp(sudoku);

                tmp.SetCell(Cell(), i, j);
                CrossHatchAll(tmp);

                if (tmp.Solve(techniques, true) == 1) {
                    changed = true;
                    sudoku.SetCell(Cell(), i, j);
                }
            }
        }
    } while (changed);
}

void CrossHatchAll(Sudoku &sudoku)
{
    for (Index_t i = 0; i < 9; ++i) {
        for (Index_t j = 0; j < 9; ++j) {
            if (sudoku.GetCell(i, j).HasValue())
                sudoku.CrossHatch(i, j);
        }
    }
}



}
