#define the compiler
CXX = mpicxx

#define compile time flags
CFLAGS = -g -O2 -std=c++17

#define the sources
SRC1 = src/generator/mazegenerator.cpp
SRC2 = src/generator/kruskal.cpp
SRC3 = src/generator/bfs.cpp
SRC4 = src/solver/dfs.cpp
SRC5 = src/solver/dijkstra.cpp
SRC6 = src/solver/mazesolver.cpp
SRC7 = src/maze.cpp

MAIN = maze.out

maze_solver : $(MAIN)
$(MAIN): $(SRC1) $(SRC2) $(SRC3) $(SRC4) $(SRC5) $(SRC6) $(SRC7)
	$(CXX) $(CFLAGS) -o $(MAIN) $(SRC1) $(SRC2) $(SRC3) $(SRC4) $(SRC5) $(SRC6) $(SRC7)

clean:
	$(RM) $(MAIN)