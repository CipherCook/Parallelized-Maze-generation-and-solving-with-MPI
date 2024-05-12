#include <iostream>
#include <vector>
#include <utility>
#include <random>
#include <mpi.h>
#include <ctime>
#include<stdlib.h>
#include "kruskal.hpp"
#include "bfs.hpp"

using namespace std;

struct cell
{
    bool right=0, bottom=0;
};

pair<int,int> getRandomEdge(int rank, int comm_sz);

int vertex_no(int i, int j);
void print2dmaze(vector<vector<char>> maze);
vector<vector<char>> convert2dmaze(vector<pair<int,int>> mst, int maze_size);
void print_1d_maze(vector<pair<int,int>> mst, int maze_size);
void maze_gen_kruskal_slave(int my_rank, int comm_sz);
void maze_gen_kruskal_master(int my_rank, int comm_sz, vector<vector<char>> &maze);
void maze_gen_bfs_slave(int my_rank, int comm_sz);
void maze_gen_bfs_master(int my_rank, int comm_sz, vector<vector<char>> &maze);
