#define the compiler
CXX = mpicxx

#define compile time flags
CFLAGS = -g -O2 -std=c++17

#define the sources
SRC1 = generator/mazegenerator.cpp
SRC2 = generator/kruskal.cpp
SRC3 = generator/bfs.cpp
SRC4 = maze.cpp

MAIN = maze.out

maze_solver : $(MAIN)
$(MAIN): $(SRC1) $(SRC2) $(SRC3) $(SRC4)
	$(CXX) $(CFLAGS) -o $(MAIN) $(SRC1) $(SRC2) $(SRC3) $(SRC4)

clean:
	$(RM) $(MAIN)