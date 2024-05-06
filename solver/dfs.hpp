#include <iostream>
#include <vector>
#include <utility>
#include <random>
#include <map>
#include <mpi.h>
#include <queue>
#include <algorithm>
#include <ctime>
#include <cstring>
#include<stdlib.h>

using namespace std;
vector<vector<char>> dfs(vector<vector<char>> maze, int rank, int comm_sz) ;
bool recursive_dfs(vector<vector<char>> &maze, vector<vector<int>> &visited, int i, int j, int rend);
