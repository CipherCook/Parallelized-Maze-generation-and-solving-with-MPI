#define the compiler
CXX = mpicxx

#define compile time flags
CFLAGS = -g -O2 -std=c++17

#define the sources
SRC1 = generator/mazegenerator.cpp
SRC2 = generator/kruskal.cpp
SRC3 = generator/bfs.cpp
SRC4 = solver/dfs.cpp
SRC5 = solver/dijkstra.cpp
SRC6 = solver/mazesolver.cpp
SRC7 = maze.cpp

MAIN = maze.out

maze_solver : $(MAIN)
$(MAIN): $(SRC1) $(SRC2) $(SRC3) $(SRC4) $(SRC5) $(SRC6) $(SRC7)
	$(CXX) $(CFLAGS) -o $(MAIN) $(SRC1) $(SRC2) $(SRC3) $(SRC4) $(SRC5) $(SRC6) $(SRC7)

clean:
	$(RM) $(MAIN)