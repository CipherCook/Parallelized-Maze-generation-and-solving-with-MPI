#include <iostream>
#include <vector>
#include <utility>
#include <random>
#include <mpi.h>
#include <ctime>
#define MAZE_SIZE 32

//mpicxx -g -O2 -std=c++17 -o maze maze.cpp

using namespace std;

int vertex_no(int i, int j) {
    return MAZE_SIZE * i + j;
}

struct cell
{
    bool right=0, bottom=0;
};

pair<int,int> getRandomEdge(int rank, int comm_sz) {
    // Define the range of rows for the given process
    int rend = std::min((rank + 1) * MAZE_SIZE / comm_sz - 1, MAZE_SIZE - 1);
    int seed = rank + (int)time(NULL);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(0, MAZE_SIZE - 1);
    // Generate and return a random row within the range
    int col =  dist(gen);
    return {vertex_no(rend, col),vertex_no(rend+1, col)} ;
}



void print2dmaze(vector<vector<char>> maze);
vector<pair<int,int>> kruskals(int rank, int comm_sz);
vector<vector<char>> convert2dmaze(vector<pair<int,int>> mst, int maze_size);
void print_1d_maze(vector<pair<int,int>> mst, int maze_size);

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

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int my_rank, comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    vector<pair<int,int>> local_mst = kruskals(my_rank, comm_sz);
    if (my_rank != 0) {
        MPI_Gather(local_mst.data(), local_mst.size() * sizeof(pair<int, int>), MPI_BYTE, nullptr, 0, MPI_BYTE, 0, MPI_COMM_WORLD);
    } else {
        int vertex_count = MAZE_SIZE * MAZE_SIZE / comm_sz;
        vector<pair<int, int>> received_vectors((vertex_count - 1) * comm_sz);
        MPI_Gather(local_mst.data(), local_mst.size() * sizeof(pair<int, int>), MPI_BYTE,
                   received_vectors.data(), local_mst.size() * sizeof(pair<int, int>), MPI_BYTE, 0, MPI_COMM_WORLD);

        // Output the merged vector
        for(int i=0; i<comm_sz-1; i++)
        {
            pair<int,int> r_edge = getRandomEdge(i, comm_sz);
            received_vectors.push_back(r_edge);
            cout<<r_edge.first<<" "<<r_edge.second<<'\n';
        }
        cout << "Merged Vector:" << endl;
        for (auto& e : received_vectors) {
            cout << "(" << e.first << ", " << e.second << ")" << " ";
        }
        cout<<"\n";
        print_1d_maze(received_vectors, MAZE_SIZE);
        cout<<"sz of received_vectors = "<<received_vectors.size()<<'\n';
        vector<vector<char>> maze = convert2dmaze(received_vectors, MAZE_SIZE);
        cout<<"sz of maze = "<<maze.size()<<" "<<maze[0].size()<<'\n';
        for (int i = 0; i < 2 * MAZE_SIZE; ++i) 
        {
            cout<<'.';
            for (int j = 0; j < 2 * MAZE_SIZE; ++j) {
                cout << maze[i][j]<<'.';
            }
            cout << "\n";
        }
    }

    MPI_Finalize();

    return 0;
}

vector<pair<int,int>> kruskals(int rank, int comm_sz) {
    int rbegin = rank * MAZE_SIZE / comm_sz;
    int rend = min((rank + 1) * MAZE_SIZE / comm_sz - 1, MAZE_SIZE - 1);
    int vertex_count = (rend - rbegin + 1) * MAZE_SIZE;

    vector<pair<int, int>> edges;
    for (int row = rbegin; row <= rend; row++) {
        for (int col = 0; col < MAZE_SIZE; col++) {
            if (row < rend) {
                edges.push_back({vertex_no(row, col), vertex_no(row + 1, col)});
            }
            if (col < MAZE_SIZE - 1) {
                edges.push_back({vertex_no(row, col), vertex_no(row, col + 1)});
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
// ```

// This should fix the issues and correctly gather the data from all processes onto process 0 for printing.

void print2dmaze(vector<vector<char>> maze)
{
    for (int i = 0; i < 2 * MAZE_SIZE; ++i) {
        cout<<'.';
        for (int j = 0; j < 2 * MAZE_SIZE; ++j) {
            cout << maze[i][j]<<'.';
        }
        cout << "\n";
    }
}



void print_1d_maze(vector<pair<int,int>> mst, int maze_size)
{
    vector<vector<char>> maze(maze_size, vector<char>(maze_size, '*'));
    struct cell grid[MAZE_SIZE][MAZE_SIZE];
    for(auto edge: mst)
    {
        int u = edge.first;
        int v = edge.second;
        int row_u = u/MAZE_SIZE;
        int col_u = u%MAZE_SIZE;
        int row_v = v/MAZE_SIZE;
        int col_v = v%MAZE_SIZE;
        // cout<<row_u<<","<<col_u<<" -> "<<row_v<<","<<col_v<<"\n";
        if(row_u == row_v)
        {
            grid[row_u][min(col_u, col_v)].right = 1;
        }
        else
        {
            grid[min(row_u, row_v)][col_u].bottom = 1;
        }
    }
    for(int i = 0; i < maze_size; i++)
    {
        for(int j = 0; j < maze_size; j++)
        {
            if(grid[i][j].bottom == 0)
            {
                cout<<"_";
            }
            else
            {
                cout<<" ";
            }
            if(grid[i][j].right == 0)
            {
                cout<<"|";
            }
            else
            {
                cout<<".";
            }
        }        
        cout<<"|\n";
    }
}


//converts a short 1-d maze to a 2d maze
vector<vector<char>> convert2dmaze(vector<pair<int,int>> mst, int maze_size)
{
    vector<vector<char>> maze(2*maze_size, vector<char>(2*maze_size, ' '));
    struct cell grid[MAZE_SIZE][MAZE_SIZE];
    for(auto edge: mst)
    {
        int u = edge.first;
        int v = edge.second;
        int row_u = u/MAZE_SIZE;
        int col_u = u%MAZE_SIZE;
        int row_v = v/MAZE_SIZE;
        int col_v = v%MAZE_SIZE;
        // cout<<row_u<<","<<col_u<<" -> "<<row_v<<","<<col_v<<"\n";
        if(row_u == row_v)
        {
            grid[row_u][min(col_u, col_v)].right = 1;
        }
        else
        {
            grid[min(row_u, row_v)][col_u].bottom = 1;
        }
    }
    for(int i=0; i<maze_size; i++)
    {
        for(int j=0; j<maze_size; j++)
        {
            maze[2*i][2*j] = '*';
            if(grid[i][j].bottom == 0)
            {
                if(i!= maze_size - 1)maze[2*i+2][2*j+1] = '*';
                // maze[2*i+1][2*j] = '*';
                
            }
            if(grid[i][j].right == 0)
            {
                // if(i>0 && grid[i-1][j].bottom == 0){
                //     maze[2*i-1][2*j+1] = '*';
                // }
                // maze[2*i][2*j+1] = '*';
                if(j!= maze_size -1) maze[2*i+1][2*j+2] = '*';
            }
        }
    }
    maze[0][2*maze_size-1] = ' ';
    maze[2*maze_size-1][0] = ' ';
    // for(int i=1; i<2*maze_size; i++)
    // {
    //     maze[i][2*maze_size-1] = '*';
    //     maze[2*maze_size-1][i] = '*';
    // }
    return maze;
}

