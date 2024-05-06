#include "generator/mazegenerator.hpp"
#include "generator/bfs.hpp"

#define MAZE_SIZE 32

//mpicxx -g -O2 -std=c++17 -o maze maze.cpp

using namespace std;

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int my_rank, comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // Parse command line arguments
    char *graph_algo = NULL;


    // Assign algorithm options
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-g") == 0) {
            graph_algo = argv[i+1];
        } 
    }
    
    // Maze Generation using Kruskal's Algorithm
    if (graph_algo != NULL && strcmp(graph_algo, "kruskal") == 0) {

        if (my_rank != 0) {
            maze_gen_kruskal_slave(my_rank, comm_sz);
        } else {
            vector<vector<char>> maze;
            maze_gen_kruskal_master(my_rank, comm_sz, maze);
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
        }
        // Apply MPI Barrier here to wait for all threads
        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}

// This should fix the issues and correctly gather the data from all processes onto process 0 for printing.





