#include "mazesolver.hpp"
#define PAR 0
#define SEQ 1
#define MAZE_SIZE 32

// This function is used to solve the maze using DFS parallely on 4 processors defined in dfs.cpp
void maze_solver_dfs(vector<vector<char>> &maze_global, int my_rank, int comm_sz){
    // This command launches the DFS Algorithm on the processors
    dfs(maze_global, my_rank, comm_sz);
}


// Dijkstra's Algorithm is implemented in dijkstra.hpp and dijkstra.cpp
// We divide the maze vertices into 4 parts using the modulo function on i * 64 + j
// This ensures uniformity over the processors.
// We then run Dijkstra's Algorithm on the 4 processors and then print the output
// The Algorithm to run Dijkstra is the same as given on Rijurekha Ma'am s webpage
// We use barriers to ensure that the printing buffer does not jumble up the output
void maze_solver_dijkstra(vector<vector<char>> &maze_global, int my_rank, int comm_sz){
    // This command launches the Dijkstra's Algorithm on the processors in parallel mode
    dijkstra(maze_global, my_rank, comm_sz, PAR);
}