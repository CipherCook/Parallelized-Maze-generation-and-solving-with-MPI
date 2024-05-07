#include "generator/mazegenerator.hpp"
#include "solver/dfs.hpp"
#define MAZE_SIZE 32

//mpicxx -g -O2 -std=c++17 -o maze maze.cpp
//mpirun -np 4 ./maze.out -g bfs -s dfs

using namespace std;

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int my_rank, comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    vector<vector<char>> maze_global(2 * MAZE_SIZE, vector<char>(2 * MAZE_SIZE));
    // Parse command line arguments
    char *graph_algo = NULL;
    char *maze_solve = NULL;

    // Assign algorithm options
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-g") == 0) {
            graph_algo = argv[i+1];
        } 
        else if (strcmp(argv[i], "-s") == 0) {
            maze_solve = argv[i+1];
        }
    }
    
    // Maze Generation using Kruskal's Algorithm
    if (graph_algo != NULL && strcmp(graph_algo, "kruskal") == 0) {
        
        if (my_rank != 0) {
            maze_gen_kruskal_slave(my_rank, comm_sz);
        } else {
            vector<vector<char>> maze;
            maze_gen_kruskal_master(my_rank, comm_sz, maze);
            maze_global = maze;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    // Maze Generation using BFS Algorithm
    if (graph_algo != NULL && strcmp(graph_algo, "bfs") == 0){
        if (my_rank != 0) {
            maze_gen_bfs_slave(my_rank, comm_sz);
        } else {
            vector<vector<char>> maze;
            maze_gen_bfs_master(my_rank, comm_sz, maze);
            maze_global = maze;
        }
        // Apply MPI Barrier here to wait for all threads
        MPI_Barrier(MPI_COMM_WORLD);

    }
    // Need to BroadCast the maze to all processes
    for (int i = 0; i < 2 * MAZE_SIZE; i++) {
        MPI_Bcast(&maze_global[i][0], 2 * MAZE_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD); //takes Only O(V) time 
    }
    MPI_Barrier(MPI_COMM_WORLD);
    vector<vector<char>> local_solved_maze = dfs(maze_global, my_rank, comm_sz);
    if(my_rank == 0){
        for(int i=0 ; i<MAZE_SIZE/2; i++){
            for(int j=0; j<2*MAZE_SIZE; j++){
                cout<<local_solved_maze[i][j];
            }
            cout<<'\n';
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(my_rank == 1){
        for(int i=0 ; i<MAZE_SIZE/2; i++){
            for(int j=0; j<2*MAZE_SIZE; j++){
                cout<<local_solved_maze[i][j];
            }
            cout<<'\n';
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(my_rank == 2){
        for(int i=0 ; i<MAZE_SIZE/2; i++){
            for(int j=0; j<2*MAZE_SIZE; j++){
                cout<<local_solved_maze[i][j];
            }
            cout<<'\n';
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(my_rank == 3){
        for(int i=0 ; i<MAZE_SIZE/2; i++){
            for(int j=0; j<2*MAZE_SIZE; j++){
                cout<<local_solved_maze[i][j];
            }
            cout<<'\n';
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    //combine all local_solved_maze to maze_global
    
    // Solve Maze using DFS Algorithm
    MPI_Finalize();

    return 0;
}
