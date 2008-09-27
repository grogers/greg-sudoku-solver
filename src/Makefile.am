bin_PROGRAMS = solver
solver_SOURCES = Logging.cpp solver.cpp Sudoku.cpp Cell.cpp \
	Techniques/Bifurcate.cpp Techniques/NakedSingle.cpp \
	Techniques/HiddenSingle.cpp Techniques/IntersectionRemoval.cpp \
	Techniques/LockedSet.cpp Techniques/Fish.cpp Techniques/BasicFish.cpp \
	Techniques/XyWing.cpp Techniques/XyzWing.cpp Techniques/UniqueRectangle.cpp \
	Sudoku.hpp Index.hpp Cell.hpp Logging.hpp Techniques.hpp Techniques/LockedSet.hpp

# Techniques should probably be made into a library...

SUBDIRS = test
