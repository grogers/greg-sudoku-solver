#include "Sudoku.hpp"
#include "Logging.hpp"

#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <boost/tokenizer.hpp>
#include <iomanip>

using namespace std;

namespace {

    struct SolverOptions {
        Sudoku::Format outputFormat;
        Sudoku::Format inputFormat;
        std::vector<Technique> techniques;
        bool bifurcate;
        bool printStatistics;

        SolverOptions()
            : outputFormat(Sudoku::Candidates), inputFormat(Sudoku::Value),
            bifurcate(false), printStatistics(true) {}
    };

    void ConvertCmdline(list<string> &, int argc, char **argv);
    void ParseOptions(const list<string> &, SolverOptions &);
    void usage();
}

int main(int argc, char **argv)
{
    list<string> cmdline;
    ConvertCmdline(cmdline, argc, argv);
    SolverOptions opts;
    ParseOptions(cmdline, opts);

    if (opts.techniques.size() == 0 && !opts.bifurcate)
        Log(Warning, "you didn't specify any techniques to use, this will only check that the puzzle is already completed\n");

    Sudoku sudoku;
    unsigned numTotal = 0, numUnique = 0, numNonUnique = 0, numImpossible = 0;

    while (sudoku.Input(cin, opts.inputFormat))
    {
        sudoku.Output(cout, opts.outputFormat); // print the read in puzzle

        int solutions = sudoku.Solve(opts.techniques, opts.bifurcate);
        if (solutions == 0) {
            ++numImpossible;
            if (opts.printStatistics)
                cout << "puzzle was impossible\n";
        } else if (solutions == 1) {
            ++numUnique;
            if (opts.printStatistics)
                cout << "puzzle was unique\n";
        } else {
            ++numNonUnique;
            if (opts.printStatistics)
                cout << "puzzle was non-unique\n";
        }

        sudoku.Output(cout, opts.outputFormat); // print the puzzle as far as it could be completed

        ++numTotal;
    }

    if (opts.printStatistics && numTotal != 0) {
        const int width = 10;
        cout << "Final Statistics:\n"
             << "-----------------\n" << left
             << "Impossible Puzzles: " << setw(width) << numImpossible << numImpossible*100/numTotal << "%\n"
             << "Non-Unique Puzzles: " << setw(width) << numNonUnique << numNonUnique*100/numTotal << "%\n"
             << "Unique Puzzles:     " << setw(width) << numUnique << numUnique*100/numTotal << "%\n"
             << "-----------------\n"
             << "Total Puzzles:      " << numTotal << '\n';
    }

    if (numTotal == numUnique)
        return 0;
    else
        return 1;
}

namespace {

void ConvertCmdline(list<string> &out, int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
        out.push_back(argv[i]);
}

void ParseOptions(const list<string> &cmdline, SolverOptions &opts)
{
    for (list<string>::const_iterator i = cmdline.begin(); i != cmdline.end(); ++i) {
        if (*i == "--help" || *i == "-h") {
            usage();
        } else if (*i == "--output-format" || *i == "-o") {
            if (++i == cmdline.end()) {
                Log(Fatal, "No argument given to option --output-format\n");
                exit(1);
            }

            if (*i == "value") {
                opts.outputFormat = Sudoku::Value;
            } else if (*i == "cand") {
                opts.outputFormat = Sudoku::Candidates;
            } else if (*i == "none") {
                opts.outputFormat = Sudoku::None;
            } else {
                Log(Fatal, "Invalid output format \'%s\' specified, expected \'value\', \'cand\', or \'none\'\n", i->c_str());
                exit(1);
            }
        } else if (*i == "--input-format" || *i == "-i") {
            if (++i == cmdline.end()) {
                Log(Fatal, "No argument given to option --input-format\n");
                exit(1);
            }

            if (*i == "value") {
                opts.inputFormat = Sudoku::Value;
            } else if (*i == "cand") {
                opts.inputFormat = Sudoku::Candidates;
            } else if (*i == "none") {
                opts.inputFormat = Sudoku::None;
            } else {
                Log(Fatal, "Invalid input format \'%s\' specified, expected \'value\', \'cand\', or \'none\'\n", i->c_str());
                exit(1);
            }
        } else if (*i == "--bifurcate" || *i == "-b") {
            opts.bifurcate = true;
        } else if (*i == "--quiet-bifurcation" || *i == "-q") {
            SetShouldQuietlyBifurcate(true);
        } else if (*i == "--log-level" || *i == "-l") {
            if (++i == cmdline.end()) {
                Log(Fatal, "No argument given to option --log-level\n");
                exit(1);
            }

            if (*i == "f" || *i == "Fatal") {
                SetLogLevel(Fatal);
            } else if (*i == "e" || *i == "Error") {
                SetLogLevel(Error);
            } else if (*i == "w" || *i == "Warning") {
                SetLogLevel(Warning);
            } else if (*i == "i" || *i == "Info") {
                SetLogLevel(Info);
            } else if (*i == "d" || *i == "Debug") {
                SetLogLevel(Debug);
            } else if (*i == "t" || *i == "Trace") {
                SetLogLevel(Trace);
            } else {
                Log(Fatal, "Invalid log level \'%s\' specified\n", i->c_str());
                usage();
            }
        } else if (*i == "--print-log-level" || *i == "-p") {
            SetShouldPrintLogLevel(true);
        } else if (*i == "--no-statistics" || *i == "-s") {
            opts.printStatistics = false;
        } else if (*i == "--techniques" || *i == "-t") {
            if (++i == cmdline.end()) {
                Log(Fatal, "No argument given to option --techniques\n");
                exit(1);
            }

            boost::char_separator<char> sep(",");
            typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
            tokenizer tokens(*i, sep);
            for (tokenizer::iterator tok = tokens.begin(); tok != tokens.end(); ++tok) {
                if (*tok == "n1") {
                    opts.techniques.push_back(&NakedSingle);
                } else if (*tok == "n2") {
                    opts.techniques.push_back(&NakedPair);
                } else if (*tok == "n3") {
                    opts.techniques.push_back(&NakedTriple);
                } else if (*tok == "n4") {
                    opts.techniques.push_back(&NakedQuad);
                } else if (*tok == "h1") {
                    opts.techniques.push_back(&HiddenSingle);
                } else if (*tok == "h2") {
                    opts.techniques.push_back(&HiddenPair);
                } else if (*tok == "h3") {
                    opts.techniques.push_back(&HiddenTriple);
                } else if (*tok == "h4") {
                    opts.techniques.push_back(&HiddenQuad);
                } else if (*tok == "lc") {
                    opts.techniques.push_back(&LockedCandidates);
                } else if (*tok == "bf2") {
                    opts.techniques.push_back(&XWing);
                } else if (*tok == "bf3") {
                    opts.techniques.push_back(&Swordfish);
                } else if (*tok == "bf4") {
                    opts.techniques.push_back(&Jellyfish);
                } else if (*tok == "ssts") {
                    opts.techniques.push_back(&SimpleSudokuTechniqueSet);
                } else if (*tok == "xyw") {
                    opts.techniques.push_back(&XyWing);
                } else if (*tok == "xyzw") {
                    opts.techniques.push_back(&XyzWing);
                } else if (*tok == "rp") {
                    opts.techniques.push_back(&RemotePair);
                } else if (*tok == "ur") {
                    opts.techniques.push_back(&UniqueRectangle);
                } else if (*tok == "fif") {
                    opts.techniques.push_back(&FinnedFish);
                } else if (*tok == "frf") {
                    opts.techniques.push_back(&FrankenFish);
                } else if (*tok == "mf") {
                    opts.techniques.push_back(&MutantFish);
                } else {
                    Log(Fatal, "Invalid argument \'%s\' given to option --techniques\n", tok->c_str());
                    usage();
                }
            }
        } else {
            Log(Fatal, "Invalid argument \'%s\' specified\n", i->c_str());
            usage();
        }
    }
}

void usage()
{
    cout << "usage: solver [options]\n"
       "options:\n"
       "    --help, -h              Print this help message.\n\n"
       "    --output-format, -o     Print sudoku's with the output format given.\n"
       "        <value|cand|none>   The default is to print candidates.\n\n"
       "    --input-format, -i      Read sudoku's in with the input format given.\n"
       "        <value|cand|none>   The default is to input by values.\n\n"
       "    --log-level, -l         Set the logging level to one of:\n"
       "        <f|e|w|i|d|t>       Fatal, Error, Warning, Info, Debug, Trace\n\n"
       "    --print-log-level, -p   Print the log level when anything is logged.\n\n"
       "    --bifurcate, -b         Use bifurcation if all other techniques fail.\n\n"
       "    --quiet-bifurcation, -q Set the log level low while bifurcating to reduce\n"
       "                            the number of spurious messages.\n\n"
       "    --no-statistics, -s     Do not print the final statistics.\n\n"
       "    --techniques, -t        Comma separated list of techniques to use, in\n"
       "        <techniques,...>    the order specified.\n"
       "                            NOTE: NakedSingle or HiddenSingle should be used\n"
       "                            first as they are the only techniques which set\n"
       "                            cells besides bifurcation.\n"
       "    Techniques:\n"
       "        ssts                simple sudoku technique set\n\n"
       "        n1, n2, n3, n4      naked single, pair, triple, quad\n"
       "        h1, h2, h3, h4      hidden single, pair, triple, quad\n"
       "        lc                  locked candidates in line/box intersections\n"
       "        bf2, bf3, bf3       x-wing, swordfish, jellyfish (\"basic fish\")\n\n"
       "        xyw                 xy-wing\n"
       "        xyzw                xyz-wing\n"
       "        rp                  remote pair\n"
       "        ur                  unique rectangle\n"
       "        fif                 finned fish (slow)\n"
       "        frf                 franken fish (very slow)\n"
       "        mf                  mutant fish (don't-even-bother slow)\n"
       "                                NOTE: this solver does not distinguish finned\n"
       "                                fish from sashimi fish\n"
       "\n"
       "return value:\n"
       "    0 - all input puzzles were completed uniquely (or there were no puzzles)\n"
       "    1 - an error occurred or not all input puzzles were unique\n"

       ;

    exit(0);
}

}
