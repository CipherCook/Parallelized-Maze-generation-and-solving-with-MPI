#include "dijkstra.hpp"
#include <set>
#include <climits>
#define MAZE_SIZE 32
#define SEQUENTIAL 0
#define PARALLEL 1

using namespace std;

int vertex_no_dijkstra(int i, int j) {
    return MAZE_SIZE * i + j;
}

// This is the parallel implementation of dijkstra's algorithm
void dijkstra(vector<vector<char>> &maze, int rank, int comm_sz, int flag) {
    int dest_row, dest_col;
    if(flag == PARALLEL){
        // Parallel Code for Maze solving using dijkstra
        // Using the same pseudo code as taught in class.
        vector<vector<int>> dist(MAZE_SIZE*2, vector<int>(2 * MAZE_SIZE, INT_MAX));
        priority_queue<
        pair<int, pair<int, int>>, // Element type
        vector<pair<int, pair<int, int>>>, // Underlying container type
        greater<pair<int, pair<int, int>>> // Comparison function
        > local_frontier;
        if(rank == 3){
            // Our start position
            dist[0][2*MAZE_SIZE-1] = 0;
            // visited[0][2*MAZE_SIZE-1] = true;
            local_frontier.push({0, {0, 2*MAZE_SIZE-1}});
        }
        MPI_Barrier(MPI_COMM_WORLD);
        int sz = (int)local_frontier.size();
        int rcv_sz;
        // Reduce the size from all elements
        MPI_Allreduce(&sz, &rcv_sz, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        sz = rcv_sz;
        // cout<<"Rank "<<rank<<" "<<sz<<'\n';
        while(sz > 0){
            // Initialize adjacency sets for all the processors
            set<pair<int,pair<int,int>>> local_adjacent_p0;
            set<pair<int,pair<int,int>>> local_adjacent_p1;
            set<pair<int,pair<int,int>>> local_adjacent_p2;
            set<pair<int,pair<int,int>>> local_adjacent_p3;

            if(!local_frontier.empty()){
                pair<int, pair<int, int>> current = local_frontier.top();
                local_frontier.pop();
                // Find the neighbours of the current node
                int i = current.second.first;
                int j = current.second.second;

                // Add the neighbours to the adjacency set
                if(i > 0 && maze[i-1][j] != '*'){   
                    int k = vertex_no_dijkstra(i-1, j) % comm_sz ;
                    if(k == 0){
                        local_adjacent_p0.insert({dist[i][j] + 1, {i-1, j}});
                    }
                    else if(k == 1){
                        local_adjacent_p1.insert({dist[i][j] + 1, {i-1, j}});
                    }
                    else if(k == 2){
                        local_adjacent_p2.insert({dist[i][j] + 1, {i-1, j}});
                    }
                    else if(k == 3){
                        local_adjacent_p3.insert({dist[i][j] + 1, {i-1, j}});
                    }
                }
                if(i < 2*MAZE_SIZE-1 && maze[i+1][j] != '*'){
                    int k = vertex_no_dijkstra(i+1, j) % comm_sz ;
                    if(k == 0){
                        local_adjacent_p0.insert({dist[i][j] + 1, {i+1, j}});
                    }
                    else if(k == 1){
                        local_adjacent_p1.insert({dist[i][j] + 1, {i+1, j}});
                    }
                    else if(k == 2){
                        local_adjacent_p2.insert({dist[i][j] + 1, {i+1, j}});
                    }
                    else if(k == 3){
                        local_adjacent_p3.insert({dist[i][j] + 1, {i+1, j}});
                    }
                }
                if(j > 0 && maze[i][j-1] != '*'){
                    int k = vertex_no_dijkstra(i, j-1) % comm_sz ;
                    if(k == 0){
                        local_adjacent_p0.insert({dist[i][j] + 1, {i, j-1}});
                    }
                    else if(k == 1){
                        local_adjacent_p1.insert({dist[i][j] + 1, {i, j-1}});
                    }
                    else if(k == 2){
                        local_adjacent_p2.insert({dist[i][j] + 1, {i, j-1}});
                    }
                    else if(k == 3){
                        local_adjacent_p3.insert({dist[i][j] + 1, {i, j-1}});
                    }
                }
                if(j < 2*MAZE_SIZE-1 && maze[i][j+1] != '*'){
                    int k = vertex_no_dijkstra(i, j+1) % comm_sz ;
                    if(k == 0){
                        local_adjacent_p0.insert({dist[i][j] + 1, {i, j+1}});
                    }
                    else if(k == 1){
                        local_adjacent_p1.insert({dist[i][j] + 1, {i, j+1}});
                    }
                    else if(k == 2){
                        local_adjacent_p2.insert({dist[i][j] + 1, {i, j+1}});
                    }
                    else if(k == 3){
                        local_adjacent_p3.insert({dist[i][j] + 1, {i, j+1}});
                    }
                }
            }
            int arr[4] = {(int)local_adjacent_p0.size(),(int)local_adjacent_p1.size(), (int)local_adjacent_p2.size(), (int)local_adjacent_p3.size()};
            MPI_Barrier(MPI_COMM_WORLD);
            // Need to first gather all the size of adjacency sets in each process
            int global_arr[16];
            MPI_Allgather(arr, 4, MPI_INT, global_arr, 4, MPI_INT, MPI_COMM_WORLD);
            // Now we need to gather all the corresponding adjacency sets in each process
            int my_sz = 0;
            if(rank == 0){
                my_sz = global_arr[0] + global_arr[4] + global_arr[8] + global_arr[12];
            }
            else if(rank == 1){
                my_sz = global_arr[1] + global_arr[5] + global_arr[9] + global_arr[13];
            }
            else if(rank == 2){
                my_sz = global_arr[2] + global_arr[6] + global_arr[10] + global_arr[14];
            }
            else if(rank == 3){
                my_sz = global_arr[3] + global_arr[7] + global_arr[11] + global_arr[15];
            }
            pair<int, pair<int, int>> global_adjacent[128];
            MPI_Barrier(MPI_COMM_WORLD);

            // Convert the sets to vectors
            vector<pair<int, pair<int, int>>> local_adjacent_p0_v(local_adjacent_p0.begin(), local_adjacent_p0.end());
            vector<pair<int, pair<int, int>>> local_adjacent_p1_v(local_adjacent_p1.begin(), local_adjacent_p1.end());
            vector<pair<int, pair<int, int>>> local_adjacent_p2_v(local_adjacent_p2.begin(), local_adjacent_p2.end());
            vector<pair<int, pair<int, int>>> local_adjacent_p3_v(local_adjacent_p3.begin(), local_adjacent_p3.end());
            // Now we need to gather all the corresponding adjacency sets in each process by broadcast the adjacency sets
            // Rank 0 Sends 1,2,3 receives and 0 updates it's own list
            if(rank == 0){
                if(global_arr[1] > 0){
                    MPI_Send(local_adjacent_p1_v.data(), global_arr[1]*3, MPI_INT, 1, 0, MPI_COMM_WORLD);
                }
                if(global_arr[2] > 0){
                    MPI_Send(local_adjacent_p2_v.data(), global_arr[2]*3, MPI_INT, 2, 0, MPI_COMM_WORLD);
                }
                if(global_arr[3] > 0){
                    MPI_Send(local_adjacent_p3_v.data(), global_arr[3]*3, MPI_INT, 3, 0, MPI_COMM_WORLD);
                }
                // Update self
                for(int i=0; i<local_adjacent_p0_v.size(); i++){
                    global_adjacent[i] = local_adjacent_p0_v[i];
                }
            }
            else{
                if(rank == 1){
                    if(global_arr[1] > 0){
                        MPI_Recv(global_adjacent, global_arr[1]*3, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }
                }
                else if(rank == 2){
                    if(global_arr[2] > 0){
                        MPI_Recv(global_adjacent, global_arr[2]*3, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }
                }
                else if(rank == 3){
                    if(global_arr[3] > 0){
                        MPI_Recv(global_adjacent, global_arr[3]*3, MPI_INT, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }
                }
            }
            MPI_Barrier(MPI_COMM_WORLD);
            // Rank 1 sends 0,2,3 receives and 1 updates it's own list
            if(rank == 1){
                if(global_arr[4] > 0){
                    MPI_Send(local_adjacent_p0_v.data(), global_arr[4]*3, MPI_INT, 0, 0, MPI_COMM_WORLD);
                }
                if(global_arr[6] > 0){
                    MPI_Send(local_adjacent_p2_v.data(), global_arr[6]*3, MPI_INT, 2, 0, MPI_COMM_WORLD);
                }
                if(global_arr[7] > 0){
                    MPI_Send(local_adjacent_p3_v.data(), global_arr[7]*3, MPI_INT, 3, 0, MPI_COMM_WORLD);
                }
                // Update self
                for(int i=0; i<local_adjacent_p1_v.size(); i++){
                    global_adjacent[i + global_arr[1]] = local_adjacent_p1_v[i];
                }
            }
            else{
                if(rank == 0){
                    if(global_arr[4] > 0){
                        MPI_Recv(global_adjacent + global_arr[0], global_arr[4]*3, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }
                }
                else if(rank == 2){
                    if(global_arr[6] > 0){
                        MPI_Recv(global_adjacent + global_arr[2], global_arr[6]*3, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }
                }
                else if(rank == 3){
                    if(global_arr[7] > 0){
                        MPI_Recv(global_adjacent + global_arr[3], global_arr[7]*3, MPI_INT, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }
                }
            }
            MPI_Barrier(MPI_COMM_WORLD);
            // Rank 2 sends 0,1,3 receives and 2 updates it's own list
            if(rank == 2){
                if(global_arr[8] > 0){
                    MPI_Send(local_adjacent_p0_v.data(), global_arr[8]*3, MPI_INT, 0, 0, MPI_COMM_WORLD);
                }
                if(global_arr[9] > 0){
                    MPI_Send(local_adjacent_p1_v.data(), global_arr[9]*3, MPI_INT, 1, 0, MPI_COMM_WORLD);
                }
                if(global_arr[11] > 0){
                    MPI_Send(local_adjacent_p3_v.data(), global_arr[11]*3, MPI_INT, 3, 0, MPI_COMM_WORLD);
                }
                // Update self
                for(int i=0; i<local_adjacent_p2_v.size(); i++){
                    global_adjacent[i + global_arr[2] + global_arr[6]] = local_adjacent_p2_v[i];
                }
            }
            else{
                if(rank == 0){
                    if(global_arr[8] > 0){
                        MPI_Recv(global_adjacent + global_arr[0] + global_arr[4], global_arr[8]*3, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }
                }
                else if(rank == 1){
                    if(global_arr[9] > 0){
                        MPI_Recv(global_adjacent + global_arr[1] + global_arr[5], global_arr[9]*3, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }
                }
                else if(rank == 3){
                    if(global_arr[11] > 0){
                        MPI_Recv(global_adjacent + global_arr[3] + global_arr[7], global_arr[11]*3, MPI_INT, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }
                }
            }
            MPI_Barrier(MPI_COMM_WORLD);
            // Rank 3 sends 0,1,2 receives and 3 updates it's own list
            if(rank == 3){
                if(global_arr[12] > 0){
                    MPI_Send(local_adjacent_p0_v.data(), global_arr[12]*3, MPI_INT, 0, 0, MPI_COMM_WORLD);
                }
                if(global_arr[13] > 0){
                    MPI_Send(local_adjacent_p1_v.data(), global_arr[13]*3, MPI_INT, 1, 0, MPI_COMM_WORLD);
                }
                if(global_arr[14] > 0){
                    MPI_Send(local_adjacent_p2_v.data(), global_arr[14]*3, MPI_INT, 2, 0, MPI_COMM_WORLD);
                }
                // Update self
                for(int i=0; i<local_adjacent_p3_v.size(); i++){
                    global_adjacent[i + global_arr[3] + global_arr[7] + global_arr[11]] = local_adjacent_p3_v[i];
                }
            }
            else{
                if(rank == 0){
                    if(global_arr[12] > 0){
                        MPI_Recv(global_adjacent + global_arr[0] + global_arr[4] + global_arr[8], global_arr[12]*3, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }
                }
                else if(rank == 1){
                    if(global_arr[13] > 0){
                        MPI_Recv(global_adjacent + global_arr[1] + global_arr[5] + global_arr[9], global_arr[13]*3, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }
                }
                else if(rank == 2){
                    if(global_arr[14] > 0){
                        MPI_Recv(global_adjacent + global_arr[2] + global_arr[6] + global_arr[10], global_arr[14]*3, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }
                }
            }
            MPI_Barrier(MPI_COMM_WORLD);
            // Now we need to update the local frontier
            for(int i=0; i<my_sz; i++){
                pair<int, pair<int, int>> current = global_adjacent[i];
                int x = current.second.first;
                int y = current.second.second;
                if(dist[x][y] > current.first){
                    dist[x][y] = current.first;
                    local_frontier.push({current.first, {x, y}});
                    // cout<<"Rank "<<rank<<" "<<x<<" "<<y<<" "<<current.first<<'\n';
                }
            }
            MPI_Barrier(MPI_COMM_WORLD);
            sz = local_frontier.size();
            MPI_Allreduce(&sz, &rcv_sz, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
            sz = rcv_sz;
        }
        // Now we need to merge the dist arrays of all the processors
        int global_dist[MAZE_SIZE*2][2*MAZE_SIZE];
        // Now we need to gather all the corresponding adjacency sets in each process
        for(int i = 0; i < 64; i++){
            for(int j = 0; j < 64; j++){
                // The processor to which the vertex belongs
                int k = vertex_no_dijkstra(i, j) % comm_sz;
                // k will share to 0
                if(k != 0){
                    if(rank == 0){
                        MPI_Recv(global_dist[i] + j, 1, MPI_INT, k, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }
                    else if(rank == k){
                        MPI_Send(&dist[i][j], 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                    }
                }
            }
        }
        // Now compute the path using the global_dist array
        dest_row = 63;
        dest_col = 0;
        int i1 = dest_row;
        int j1 = dest_col;
        while(i1 != 0 || j1 != 0){
            maze[i1][j1] = 'P';
            if(i1 > 0 && global_dist[i1-1][j1] == global_dist[i1][j1] - 1){
                i1 = i1 - 1;
            }
            else if(i1 < 2*MAZE_SIZE - 1 && global_dist[i1+1][j1] == global_dist[i1][j1] - 1){
                i1 = i1 + 1;
            }
            else if(j1 > 0 && global_dist[i1][j1-1] == global_dist[i1][j1] - 1){
                j1 = j1 - 1;
            }
            else if(j1 < 2*MAZE_SIZE - 1 && global_dist[i1][j1+1] == global_dist[i1][j1] - 1){
                j1 = j1 + 1;
            }
        }
        maze[0][63] = 'S';
        maze[63][0] = 'E';
        MPI_Barrier(MPI_COMM_WORLD);
        // Print the matrix
        if(rank == 0){
            for(int i=0 ; i<2*MAZE_SIZE; i++){
                for(int j=0; j<2*MAZE_SIZE; j++){
                    cout<<maze[i][j];
                }
                if(i!=2*MAZE_SIZE-1) cout<<'\n';
            }
        }
        return;
    }
    else{
        // Sequential Code for Maze solving using dijkstra for comparison purposes
        // Need to solve the maze using dijkstra's algorithm
        int rbegin = rank *2* MAZE_SIZE / comm_sz;
        int rend = min((rank + 1) * 2*MAZE_SIZE / comm_sz - 1, 2*MAZE_SIZE - 1);
        vector<vector<char>> local_solved_maze(MAZE_SIZE/2, vector<char>(2 * MAZE_SIZE)); 
        for(int r=rbegin; r<=rend; r++){
            for(int c=0; c<2*MAZE_SIZE; c++){
                local_solved_maze[r-rbegin][c] = maze[r][c];
            }
        }
        dest_row = (2*MAZE_SIZE)/comm_sz - 1;
        if(rank == 3){
            dest_col = 0;
        }
        else{
            int c = 63;
            for(int j=0; j<2*MAZE_SIZE; j++){
                if(maze[rend + 1][j] == ' '){
                    c = j;
                    break;
                }
            }
            dest_col = c;
        }
        int c = 63;
        if(rbegin != 0){
            for(int j=0; j<2*MAZE_SIZE; j++){
                if(local_solved_maze[0][j] == ' '){
                    c = j;
                    break;
                }
            }
        }
        // Run a sequential dijkstra's algorithm from (0,c) to (dest_row, dest_col)
        // Make a distance array
        vector<vector<int>> dist(MAZE_SIZE/2, vector<int>(2 * MAZE_SIZE, INT_MAX));
        // Now start the code
        dist[0][c] = 0;
        // For weight equal to 1 the dijkstra is same as BFS therefore for all real world purposes
        // we can use BFS for this problem for all time comparisons
        queue<pair<int, int>> frontier;
        frontier.push({0, c});
        int level = 1;
        while(!frontier.empty()){
            queue<pair<int, int>> next_frontier;
            while(!frontier.empty()){
                pair<int, int> current = frontier.front();
                frontier.pop();
                int x = current.first;
                int y = current.second;
                if(x == dest_row && y == dest_col){
                    break;
                }
                if(x > 0 && local_solved_maze[x-1][y] == ' ' && dist[x-1][y] == INT_MAX){
                    dist[x-1][y] = level;
                    next_frontier.push({x-1, y});
                }
                if(x < MAZE_SIZE/2 - 1 && local_solved_maze[x+1][y] == ' ' && dist[x+1][y] == INT_MAX){
                    dist[x+1][y] = level;
                    next_frontier.push({x+1, y});
                }
                if(y > 0 && local_solved_maze[x][y-1] == ' ' && dist[x][y-1] == INT_MAX){
                    dist[x][y-1] = level;
                    next_frontier.push({x, y-1});
                }
                if(y < 2*MAZE_SIZE - 1 && local_solved_maze[x][y+1] == ' ' && dist[x][y+1] == INT_MAX){
                    dist[x][y+1] = level;
                    next_frontier.push({x, y+1});
                }
            }
            level++;
            frontier = next_frontier;
        }
        // Now we need to update the local_solved_maze recursively
        int i1 = dest_row;
        int j1 = dest_col;
        while(i1 != 0 || j1 != c){
            local_solved_maze[i1][j1] = 'P';
            if(i1 > 0 && dist[i1-1][j1] == dist[i1][j1] - 1){
                i1 = i1 - 1;
            }
            else if(i1 < MAZE_SIZE/2 - 1 && dist[i1+1][j1] == dist[i1][j1] - 1){
                i1 = i1 + 1;
            }
            else if(j1 > 0 && dist[i1][j1-1] == dist[i1][j1] - 1){
                j1 = j1 - 1;
            }
            else if(j1 < 2*MAZE_SIZE - 1 && dist[i1][j1+1] == dist[i1][j1] - 1){
                j1 = j1 + 1;
            }
        }
        local_solved_maze[0][c] = 'P';
        // Now we need to return the local_solved_maze
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank == 0){
            local_solved_maze[0][63] = 'S';
            for(int i=0 ; i<MAZE_SIZE/2; i++){
                for(int j=0; j<2*MAZE_SIZE; j++){
                    cout<<local_solved_maze[i][j];
                }
                cout<<'\n';
            }
        }
        // Printing the local matrix stored at processor 1
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank == 1){
            for(int i=0 ; i<MAZE_SIZE/2; i++){
                for(int j=0; j<2*MAZE_SIZE; j++){
                    cout<<local_solved_maze[i][j];
                }
                cout<<'\n';
            }
        }
        // Printing the local matrix stored at processor 2
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank == 2){
            for(int i=0 ; i<MAZE_SIZE/2; i++){
                for(int j=0; j<2*MAZE_SIZE; j++){
                    cout<<local_solved_maze[i][j];
                }
                cout<<'\n';
            }
        }
        // Printing the local matrix stored at processor 3
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank == 3){
            local_solved_maze[MAZE_SIZE/2-1][0] = 'E';
            for(int i=0 ; i<MAZE_SIZE/2; i++){
                for(int j=0; j<2*MAZE_SIZE; j++){
                    cout<<local_solved_maze[i][j];
                }
                if(i!=MAZE_SIZE/2-1){
                    cout<<'\n';
                }
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    return ;
}