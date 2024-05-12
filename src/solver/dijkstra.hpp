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

void dijkstra(vector<vector<char>> &maze, int rank, int comm_sz , int flag);