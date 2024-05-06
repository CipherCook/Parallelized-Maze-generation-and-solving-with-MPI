#include "generator/mazegenerator.hpp"
#include "generator/kruskal.hpp"
#include "generator/bfs.hpp"

#define MAZE_SIZE 32

// Make maze
// mpirun -np 4 ./maze.out -g kruskal/bfs

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
    if (graph_algo != NULL && strcmp(graph_algo, "kruskal") == 0) {
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
    }
    if (graph_algo != NULL && strcmp(graph_algo, "bfs") == 0){
        vector<pair<int,int>> local_mst = bfs(my_rank, comm_sz);
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
    }
    MPI_Finalize();

    return 0;
}

// This should fix the issues and correctly gather the data from all processes onto process 0 for printing.





