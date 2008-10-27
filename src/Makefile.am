SUBDIRS = test techniques boost

SOURCES = Logging.cpp Sudoku.cpp Cell.cpp \
	Sudoku.hpp Index.hpp Cell.hpp Logging.hpp Techniques.hpp DefineGroup.hpp
LDADD = techniques/libtechniques.a

bin_PROGRAMS = solver generator
solver_SOURCES = solver.cpp ${SOURCES}
generator_SOURCES = generator.cpp ${SOURCES}
