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
    void ConvertCmdline(list<string> &, int argc, char **argv);
    void ParseOptions(const list<string> &);
    void usage();

    Sudoku::Format outputFormat = Sudoku::Candidates;
    std::vector<Technique> techniques;
    bool bifurcate = false;
}

int main(int argc, char **argv)
{
    list<string> cmdline;
    ConvertCmdline(cmdline, argc, argv);
    ParseOptions(cmdline);

    if (techniques.size() == 0 && !bifurcate) {
        Log(Fatal, "You must specify at least one technique or use bifurcation!\n");
        exit(1);
    }

    Sudoku sudoku;
    unsigned numTotal = 0, numUnique = 0, numNonUnique = 0, numImpossible = 0;

    while (sudoku.Input(cin))
    {
        sudoku.Output(cout, outputFormat); // print the read in puzzle

        int solutions = sudoku.Solve(techniques);
        if (solutions == 1) {
            cout << "puzzle was solved uniquely!\n";
            ++numUnique;
        } else {
            cout << "puzzle had " << solutions << " solutions\n";
            if (solutions > 1)
                ++numNonUnique;
            else
                ++numImpossible;
        }

        sudoku.Output(cout, outputFormat); // print the puzzle as far as it could be completed

        ++numTotal;
    }

    if (numTotal != 0) {
        const int width = 10;
        cout << "Final Statistics:\n"
             << "-----------------\n" << left
             << "Impossible Puzzles: " << setw(width) << numImpossible << numImpossible*100/numTotal << "%\n"
             << "Non-Unique Puzzles: " << setw(width) << numNonUnique << numNonUnique*100/numTotal << "%\n"
             << "Unique Puzzles:     " << setw(width) << numUnique << numUnique*100/numTotal << "%\n"
             << "-----------------\n"
             << "Total Puzzles:      " << numTotal << '\n';
    }

	return 0;
}

bool UseBifurcation()
{
    return bifurcate;
}

namespace {

void ConvertCmdline(list<string> &out, int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
        out.push_back(argv[i]);
}

void ParseOptions(const list<string> &opts)
{
    for (list<string>::const_iterator i = opts.begin(); i != opts.end(); ++i) {
        if (*i == "--help" || *i == "-h") {
            usage();
        } else if (*i == "--output-format" || *i == "-o") {
            ++i;
            if (i == opts.end()) {
                Log(Fatal, "No argument given to option --output-format\n");
                exit(1);
            }

            if (*i == "value") {
                outputFormat = Sudoku::Value;
            } else if (*i == "cand") {
                outputFormat = Sudoku::Candidates;
            } else if (*i == "none") {
                outputFormat = Sudoku::None;
            } else {
                Log(Fatal, "Invalid output format \'%s\' specified, expected \'value\', \'cand\', or \'none\'\n", i->c_str());
                exit(1);
            }
        } else if (*i == "--bifurcate" || *i == "-b") {
            bifurcate = true;
        } else if (*i == "--log-level" || *i == "-l") {
            ++i;
            if (i == opts.end()) {
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
                Log(Fatal, "Invalid output format \'%s\' specified\n", i->c_str());
                usage();
            }
        } else if (*i == "--print-log-level" || *i == "-p") {
            SetShouldPrintLogLevel(true);
        } else if (*i == "--techniques" || *i == "-t") {
            ++i;
            if (i == opts.end()) {
                Log(Fatal, "No argument given to option --techniques\n");
                exit(1);
            }

            boost::char_separator<char> sep(",");
            typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
            tokenizer tokens(*i, sep);
            for (tokenizer::iterator tok = tokens.begin(); tok != tokens.end(); ++tok) {
                if (*tok == "n1" || *tok == "NakedSingle") {
                    techniques.push_back(&NakedSingle);
                } else if (*tok == "h1" || *tok == "HiddenSingle") {
                    techniques.push_back(&HiddenSingle);
                } else if (*tok == "ir" || *tok == "IntersectionRemoval") {
                    techniques.push_back(&IntersectionRemoval);
                } else if (*tok == "ns" || *tok == "NakedSet") {
                    techniques.push_back(&NakedSet);
                } else if (*tok == "hs" || *tok == "HiddenSet") {
                    techniques.push_back(&HiddenSet);
                } else {
                    Log(Fatal, "Invalied argument \'%s\' given to option --techniques\n", tok->c_str());
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
       "    --help, -h                  Print this help message.\n\n"
       "    --output-format, -o         Print sudoku's with the output format given.\n"
       "        <value|cand|none>       The default is to print candidates.\n\n"
       "    --bifurcate, -b             Use bifurcation if all other techniques fail.\n\n"
       "    --log-level, -l             Set the logging level to one of:\n"
       "        <f|e|w|i|d|t>           Fatal, Error, Warning, Info, Debug, Trace\n\n"
       "    --print-log-level, -p       Print the log level when anything is logged.\n"
       "    --techniques, -t            Comma separated list of techniques to use, in\n"
       "        <techniques,...>        the order specified.\n"
       "                                NOTE: NakedSingle or HiddenSingle should be used\n"
       "                                first as they are the only techniques which set\n"
       "                                cells besides bifurcation.\n"
       "    Techniques:\n"
       "        n1, NakedSingle         Uses naked singles\n"
       "        h1, HiddenSingle        Uses hidden singles\n"
       "        ir, IntersectionRemoval Uses intersections between houses\n"
       "        ns. NakedSet            Uses naked sets of order 2-4\n"
       "        hs, HiddenSet           Uses hidden sets of order 2-4\n"
       ;

    exit(0);
}

}
