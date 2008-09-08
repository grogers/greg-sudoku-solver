
void ParseOptions(int argc, char **argv);

int main(int argc, char **argv)
{
	ParseOptions(argc, argv);

	// keep trying to read sudoku's from standard input and solve them
	for (;;)
	{
		bool solved;
		Sudoku puzzle(verbose_level);

		std::cin >> puzzle;
		std::cout << puzzle;

		if (solve_fast)
			solved = puzzle.FastSolve();
		else
			solved = puzzle.Solve();

		if (solved)
			std::cout << "Hooray Puzzle Was Solved!!!\n";
		else
			std::cout << "Sorry Puzzle Wasn't Solved...\n";
	
		std::cout << puzzle;
	}

	return 0;
}

void ParseOptions(int argc, char **argv)
{
	boost::program_options::options_description desc("Allowed options");

	desc.add_options()
		("help,h", "produce this help message")
		("verbose-level,v", 
			boost::program_options::value<int>()->default_value(0),
			"set how verbose the program should be")
		("fast,f", "solve as fast as possible, but use bifurcation")
	;

	boost::program_options::variables_map vm;
	boost::program_options::store(
		boost::program_options::parse_command_line(argc, argv, desc), vm);

	boost::program_options::notify(vm);    

	if (vm.count("help"))
	{
		std::cout << desc << "\n";
		exit(0);
	}

	if (vm.count("verbose-level"))
		verbose_level = vm["verbose-level"].as<int>();

	if (vm.count("fast"))
		solve_fast = true;
}
