#include <iostream>
#include <vector>
#include <utility>
#include <random>
#include <map>
#include <mpi.h>
#include <ctime>
#include<stdlib.h>

using namespace std;
vector<pair<int,int>> bfs(int rank, int comm_sz);
