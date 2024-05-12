#include "dfs.hpp"
#include "dijkstra.hpp"

void maze_solver_dfs(vector<vector<char>> &maze_global, int my_rank, int comm_sz);
void maze_solver_dijkstra(vector<vector<char>> &maze_global, int my_rank, int comm_sz);