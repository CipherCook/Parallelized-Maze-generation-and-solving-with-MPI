#include "bfs.hpp"
#define MAZE_SIZE 32
#define VISIT 42
using namespace std;

int vertex_no_bfs(int i, int j) {
    return MAZE_SIZE * i + j;
}
vector<pair<int,int>> bfs(int rank, int comm_sz){
    int rbegin = rank * MAZE_SIZE / comm_sz;
    int rend = min((rank + 1) * MAZE_SIZE / comm_sz - 1, MAZE_SIZE - 1);
    int vertex_count = (rend - rbegin + 1) * MAZE_SIZE;

    vector<pair<int, int>> edges;
    map<int,int> visited;

    random_device rd;
    int seed = rank + (int)time(NULL);
    mt19937 g(rd()+seed);
    queue<int> q;
    // CHOOSE A RANDOM SOURCE FOR THE BFS
    std::uniform_int_distribution<int> dis_col(0, MAZE_SIZE - 1);
    std::uniform_int_distribution<int> dist_row(rbegin, rend);
    int source = vertex_no_bfs(dist_row(g), dis_col(g));
    q.push(source);
    visited[source]=VISIT;
    while(!q.empty()){
        queue<int> r;
        while(!q.empty()){
            int u = q.front();
            q.pop();
            vector<int> next_frontier;
            int row = u/MAZE_SIZE;
            int col = u%MAZE_SIZE;
            if(row>rbegin){
                if(visited[vertex_no_bfs(row-1,col)]!=VISIT){
                    next_frontier.push_back(vertex_no_bfs(row-1,col));
                    edges.push_back({u,vertex_no_bfs(row-1,col)});
                    visited[vertex_no_bfs(row-1,col)]=VISIT;
                }
            }
            if(row<rend){
                if(visited[vertex_no_bfs(row+1,col)]!=VISIT){
                    next_frontier.push_back(vertex_no_bfs(row+1,col));
                    edges.push_back({u,vertex_no_bfs(row+1,col)});
                    visited[vertex_no_bfs(row+1,col)]=VISIT;
                }
            }
            if(col>0){
                if(visited[vertex_no_bfs(row,col-1)]!=VISIT){
                    next_frontier.push_back(vertex_no_bfs(row,col-1));
                    edges.push_back({u,vertex_no_bfs(row,col-1)});
                    visited[vertex_no_bfs(row,col-1)]=VISIT;
                }
            }
            if(col<MAZE_SIZE-1){
                if(visited[vertex_no_bfs(row,col+1)]!=VISIT){
                    next_frontier.push_back(vertex_no_bfs(row,col+1));
                    edges.push_back({u,vertex_no_bfs(row,col+1)});
                    visited[vertex_no_bfs(row,col+1)]=VISIT;
                }
            }
            // Add to r
                for(auto v:next_frontier){
                    r.push(v);
                }
        }
        // Shuffle and shift r to q
        vector<int> frontier;
        while(!r.empty()){
            frontier.push_back(r.front());
            r.pop();
        }
        shuffle(frontier.begin(), frontier.end(), g);
        for(auto v:frontier){
            q.push(v);
        }
    }

    vector<pair<int, int>> mst;
    // Push all the edges selected into the MST
    for(auto edge:edges){
        mst.push_back(edge);
    }
    cout << "Process " << rank << " has " << mst.size() << " edges in its MST" << endl;
    return mst;
}
