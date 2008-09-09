#include "Sudoku.hpp"
#include "Logging.hpp"

#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <boost/tokenizer.hpp>

using namespace std;

namespace {
    void ConvertCmdline(list<string> &, int argc, char **argv);
    void ParseOptions(const list<string> &);
    void usage();

    Sudoku::Format outputFormat = Sudoku::Candidates;
    std::vector<Technique> techniques(1, &NakedSingle);
    bool bifurcate = false;
}

int main(int argc, char **argv)
{
    list<string> cmdline;
    ConvertCmdline(cmdline, argc, argv);
    ParseOptions(cmdline);

    if (techniques.size() == 0) {
        Log(Fatal, "You must specify at least one technique!\n");
        exit(1);
    }

    Sudoku sudoku;

    while (sudoku.Input(cin))
    {
        sudoku.Output(cout, outputFormat); // print the read in puzzle

        int solutions = sudoku.Solve(techniques);
        if (solutions == 1)
            cout << "puzzle was solved uniquely!\n";
        else
            cout << "puzzle had " << solutions << " solutions\n";

        sudoku.Output(cout, outputFormat); // print the puzzle as far as it could be completed
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
                if (*tok == "ns" || *tok == "NakedSingle") {
                    techniques.push_back(&NakedSingle);
                } else if (*tok == "hs" || *tok == "HiddenSingle") {
                    techniques.push_back(&HiddenSingle);
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
       "    --help, -h                  Print this help message\n"
       "    --output-format, -o         Print sudoku's with the output format given.\n"
       "        <value|cand|none>       The default is to print candidates.\n"
       "    --bifurcate, -b             Use bifurcation if all other techniques fail\n"
       "    --techniques, -t            Comma separated list of techniques to use, in\n"
       "        <techniques,...>        the order specified.\n"
       "                                NOTE: NakedSingle is always applied as the\n"
       "                                first technique, regardless of what is specified\n"
       "                                on the command line.\n"
       "    Techniques:\n"
       "        ns, NakedSingle         Uses naked singles\n"
       "        hs, HiddenSingle        Uses hidden singles\n"
       ;

    exit(0);
}

}
