#include <iostream>
#include <vector>
#include <utility>
#include <random>
#include <mpi.h>
#include <ctime>
#include<stdlib.h>

using namespace std;

class DSU {
private:
    vector<int> parent;
    vector<int> rank;

public:
    DSU(int n) {
        parent.resize(n);
        rank.resize(n, 0);
        for (int i = 0; i < n; ++i) {
            parent[i] = i;  // Each node is its own parent initially
        }
    }

    int find(int u) {
        if (parent[u] != u) {
            parent[u] = find(parent[u]);  // Path compression
        }
        return parent[u];
    }

    void union_set(int u, int v) {
        int pu = find(u);
        int pv = find(v);
        if (pu == pv) return;  // Already in the same set

        // Union by rank
        if (rank[pu] > rank[pv]) {
            parent[pv] = pu;
        } else if (rank[pu] < rank[pv]) {
            parent[pu] = pv;
        } else {
            parent[pv] = pu;
            rank[pu]++;
        }
    }
};


vector<pair<int,int>> kruskals(int rank, int comm_sz);
