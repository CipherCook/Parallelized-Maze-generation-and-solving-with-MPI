#include "kruskal.hpp"
#define MAZE_SIZE 32
using namespace std;

int vertex_no_kruskal(int i, int j) {
    return MAZE_SIZE * i + j;
}

vector<pair<int,int>> kruskals(int rank, int comm_sz) {
    int rbegin = rank * MAZE_SIZE / comm_sz;
    int rend = min((rank + 1) * MAZE_SIZE / comm_sz - 1, MAZE_SIZE - 1);
    int vertex_count = (rend - rbegin + 1) * MAZE_SIZE;

    vector<pair<int, int>> edges;
    for (int row = rbegin; row <= rend; row++) {
        for (int col = 0; col < MAZE_SIZE; col++) {
            if (row < rend) {
                edges.push_back({vertex_no_kruskal(row, col), vertex_no_kruskal(row + 1, col)});
            }
            if (col < MAZE_SIZE - 1) {
                edges.push_back({vertex_no_kruskal(row, col), vertex_no_kruskal(row, col + 1)});
            }
        }
    }

    random_device rd;
    mt19937 g(rd());
    shuffle(edges.begin(), edges.end(), g);

    DSU dsu(MAZE_SIZE * MAZE_SIZE);
    vector<pair<int, int>> mst;
    int add_cnt = 0;

    for (auto edge : edges) {
        int u = edge.first;
        int v = edge.second;
        if (dsu.find(u) != dsu.find(v)) {
            dsu.union_set(u, v);
            mst.push_back(edge);
            add_cnt++;
        }
        if (add_cnt == vertex_count - 1) {
            break;
        }
    }
    cout << "Process " << rank << " has " << mst.size() << " edges in its MST" << endl;
    return mst;
}