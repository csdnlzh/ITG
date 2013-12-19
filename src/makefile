# Executable
EXE    = itg

# Compiler, Linker Defines
CC      = g++
PG      = #-g #-pg
CFLAGS  = -Wall -Ofast -Wno-deprecated   $(PG) -std=c++0x -pthread 
LIBS    = liblbfgs.a 

	
# Compile and Assemble C++ Source Files into Object Files
#echo compiling...
%.o: %.cpp
	$(CC) -c $(CFLAGS) $<
# -o $@
# Source and Object files
SRC    = $(wildcard *.cpp)
OBJ    = $(patsubst %.cpp, %.o, $(SRC))

# Link all Object Files with external Libraries into Binaries
#echo linking...

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ)  -o $(EXE) -lz $(LIBS) -lncurses

.PHONY: clean
clean:
	 -rm -f core *.o
