SUBDIRS = test techniques

bin_PROGRAMS = solver
solver_SOURCES = Logging.cpp solver.cpp Sudoku.cpp Cell.cpp \
	Sudoku.hpp Index.hpp Cell.hpp Logging.hpp Techniques.hpp
solver_LDADD = techniques/libtechniques.a
