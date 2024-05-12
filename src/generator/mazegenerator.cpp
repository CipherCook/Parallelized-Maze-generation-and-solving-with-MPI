#include "mazegenerator.hpp"

#define MAZE_SIZE 32

using namespace std;

int vertex_no(int i, int j) {
    return MAZE_SIZE * i + j;
}

// Function to generate a random edge for the given process
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

// converts a short 1-d tree to a 2d maze (given a MST of compressed maze we can convert it to maze of double size)
// We can use this function to convert the 1d tree to 2d maze with all the paths/walks on tree mapped
// to the 2d maze.
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
                
            }
            if(grid[i][j].right == 0)
            {
                if(j!= maze_size -1) maze[2*i+1][2*j+2] = '*';
            }
        }
    }
    maze[0][2*maze_size-1] = ' ';
    maze[2*maze_size-1][0] = ' ';
    return maze;
}

// ALGORITHM IDEA : TO GENERATE A MATRIX OF SIZE 64 * 64
// We need an MST of size 32 * 32
// With all the paths of MST as a walk in the 64 * 64 matrix
// We can use the above function to convert the MST to a 64 * 64 matrix

// To do so parallely we can divide the vertices into equal parts
// We do so by using 
// rank 0 -> 0-15  rows
// rank 1 -> 16-31 rows
// rank 2 -> 32-47 rows
// rank 3 -> 48-63 rows
// Join them to make a forest
// We then choose 3 random edges amongst them and add them to the forest to make a tree
// We use the same idea in both Kruskal's and BFS 
// The method that changes is the generation of the MST 
// Kruskal's uses a DSU to generate the MST 
// BFS uses a random BFS traversal to generate the MST



// In order to parallelize the Kruskal's Algo we need to divide the vertices into equal parts
// We use contigous rows for generation of a MST of half the size of the maze
void maze_gen_kruskal_slave(int my_rank, int comm_sz){
    // Generation of MST on the slave processors 1,2,3
    vector<pair<int,int>> local_mst = kruskals(my_rank, comm_sz);
    // Once done we need to send the local MST to the master
    MPI_Gather(local_mst.data(), local_mst.size() * sizeof(pair<int, int>), MPI_BYTE, nullptr, 0, MPI_BYTE, 0, MPI_COMM_WORLD);
}

void maze_gen_kruskal_master(int my_rank, int comm_sz, vector<vector<char>> &maze){
    // Generation of MST on the master processor 0
    vector<pair<int,int>> local_mst = kruskals(my_rank, comm_sz);
    int vertex_count = MAZE_SIZE * MAZE_SIZE / comm_sz;
    vector<pair<int, int>> received_vectors((vertex_count - 1) * comm_sz);
    // Gather the local MST from all the processors
    MPI_Gather(local_mst.data(), local_mst.size() * sizeof(pair<int, int>), MPI_BYTE,
            received_vectors.data(), local_mst.size() * sizeof(pair<int, int>), MPI_BYTE, 0, MPI_COMM_WORLD);
    // // Output the merged vector
    for(int i=0; i<comm_sz-1; i++)
    {
        pair<int,int> r_edge = getRandomEdge(i, comm_sz);
        received_vectors.push_back(r_edge);
    }
    maze = convert2dmaze(received_vectors, MAZE_SIZE);
}

// In order to parallelize the BFS Algo we need to divide the vertices into equal parts
// We use contigous rows for generation of a MST of half the size of the maze
void maze_gen_bfs_slave(int my_rank, int comm_sz){
    // Generation of MST on the slave processors 1,2,3
    vector<pair<int,int>> local_mst = bfs(my_rank, comm_sz);
    // Once done we need to send the local MST to the master
    MPI_Gather(local_mst.data(), local_mst.size() * sizeof(pair<int, int>), MPI_BYTE, nullptr, 0, MPI_BYTE, 0, MPI_COMM_WORLD);
}

void maze_gen_bfs_master(int my_rank, int comm_sz, vector<vector<char>> &maze){
    // Generation of MST on the master processor 0
    vector<pair<int,int>> local_mst = bfs(my_rank, comm_sz);
    int vertex_count = MAZE_SIZE * MAZE_SIZE / comm_sz;
    vector<pair<int, int>> received_vectors((vertex_count - 1) * comm_sz);
    // Gather the local MST from all the processors
    MPI_Gather(local_mst.data(), local_mst.size() * sizeof(pair<int, int>), MPI_BYTE,
            received_vectors.data(), local_mst.size() * sizeof(pair<int, int>), MPI_BYTE, 0, MPI_COMM_WORLD);

    // Output the merged vector
    for(int i=0; i<comm_sz-1; i++)
    {
        pair<int,int> r_edge = getRandomEdge(i, comm_sz);
        received_vectors.push_back(r_edge);
    }
    maze = convert2dmaze(received_vectors, MAZE_SIZE);
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