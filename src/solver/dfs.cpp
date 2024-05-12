#include "dfs.hpp"
#include <stack>
#include <climits>
#define MAZE_SIZE 32
using namespace std;

int dest_row, dest_col;

// This is a function to recursively solve the maze using DFS in parallel
// Algorithm : Similar to that in Pacheco pg 328
// We start the work on a single processor and after every iteration if some processor is free
// then the top of the stack is popped from the processor with the most work and given to the free processor
// We use it to update the parent map
void dfs_parallel(vector<vector<char>> &maze, vector<vector<int>> &visited, int rank, int comm_sz){
    stack<pair<int, int>> s;
    map<pair<int, int>, pair<int, int>> parent;
    // Now we need to run the dfs on the local maze until we reach (63,0).
    // We will keep track of the parent of each cell in the maze
    if(rank == 0){
        s.push({0, 63});
        visited[0][63] = 1;
        parent[{0, 63}] = {-1, -1};
    }
    bool reached = false;
    while(!reached){
        // First we need to use all to all to see who has the max size and is there anyone with stack
        // size equal to zero
        int stack_size = s.size();
        int arr_sz[4];
        MPI_Allgather(&stack_size, 1, MPI_INT, arr_sz, 1, MPI_INT, MPI_COMM_WORLD);
        int max_sz = 0;
        int max_proc = -1;
        for(int i=0; i<4; i++){
            if(arr_sz[i] > max_sz){
                max_sz = arr_sz[i];
                max_proc = i;
            }
        }
        // If two have max sz then we choose the leftmost one
        int free_proc = -1;
        for(int i=0; i<4; i++){
            if(arr_sz[i] == 0){
                free_proc = i;
                break;
            }
        }
        if(free_proc != -1){
            // Now we need to send the top of the stack to the free processor
            if(rank == free_proc){
                // Need to receive the top of the stack from the processor with max stack size
                pair<int, int> top;
                MPI_Recv(&top, 2, MPI_INT, max_proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                s.push(top);
            }
            if(rank == max_proc){
                // Need to send the top of the stack to the free processor
                pair<int, int> top = s.top();
                s.pop();
                MPI_Send(&top, 2, MPI_INT, free_proc, 0, MPI_COMM_WORLD);
            }
            // Wait for all processors to reach this point
            MPI_Barrier(MPI_COMM_WORLD);
        }
        // Now we need to run the dfs on the local maze for one iteration
        pair<int,int> node = s.top();
        s.pop();
        int i = node.first;
        int j = node.second;
        if(i > 0 && !visited[i-1][j] && maze[i-1][j] == ' '){
            visited[i-1][j] = 1;
            s.push({i-1, j});
            parent[{i-1, j}] = {i, j};
        }
        if(i < 63 && !visited[i+1][j] && maze[i+1][j] == ' '){
            visited[i+1][j] = 1;
            s.push({i+1, j});
            parent[{i+1, j}] = {i, j};
        }
        if(j > 0 && !visited[i][j-1] && maze[i][j-1] == ' '){
            visited[i][j-1] = 1;
            s.push({i, j-1});
            parent[{i, j-1}] = {i, j};
        }
        if(j < 63 && !visited[i][j+1] && maze[i][j+1] == ' '){
            visited[i][j+1] = 1;
            s.push({i, j+1});
            parent[{i, j+1}] = {i, j};
        }
        // Now we need to wait for all processors to reach this point
        MPI_Barrier(MPI_COMM_WORLD);
        // Now regarding the termination condition
        if(visited[63][0] == 1){
            reached = true;
        }
        // Use MPI_Allreduce to check if any processor has reached the destination
        int reached_int = reached ? 1 : 0;
        int global_reached;
        MPI_Allreduce(&reached_int, &global_reached, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    }
    // Now we need to backtrack to find the path
    // For which we need to broadcast entire parent map to all processors
    map<pair<int, int>, pair<int, int>> global_parent;
    // First we need to all know the size of the parent maps in each process
    int parent_size = parent.size();
    int arr_parent_size[4];
    MPI_Allgather(&parent_size, 1, MPI_INT, arr_parent_size, 1, MPI_INT, MPI_COMM_WORLD);
    // Now we need to send the parent map to the root processor
    if(rank == 0){
        // Recv from p1
        vector<pair<pair<int,int>,pair<int, int>>> arr(arr_parent_size[1]);
        MPI_Recv(arr.data(), arr_parent_size[1], MPI_2INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for(int i=0; i<arr_parent_size[1]; i++){
            global_parent[arr[i].first] = arr[i].second;
        }
        // Recv from p2
        vector<pair<pair<int,int>,pair<int, int>>> arr2(arr_parent_size[2]);
        MPI_Recv(arr2.data(), arr_parent_size[2], MPI_2INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for(int i=0; i<arr_parent_size[2]; i++){
            global_parent[arr2[i].first] = arr2[i].second;
        }
        // Recv from p3
        vector<pair<pair<int,int>,pair<int, int>>> arr3(arr_parent_size[3]);
        MPI_Recv(arr3.data(), arr_parent_size[3], MPI_2INT, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for(int i=0; i<arr_parent_size[3]; i++){
            global_parent[arr3[i].first] = arr3[i].second;
        }
    }
    else{
        // Send the parent map to the root processor
        int sz = parent.size();
        vector<pair<pair<int,int>,pair<int, int>>> arr(sz);
        int i = 0;
        for(auto it=parent.begin(); it!=parent.end(); it++){
            arr[i] = {it->first, it->second};
            i++;
        }
        MPI_Send(arr.data(), sz, MPI_2INT, 0, 0, MPI_COMM_WORLD);
    }
    // Now the root processor has the global parent map
    // Now we need to backtrack to find the path and update the maze
    if(rank == 0){
        pair<int, int> node = {63, 0};
        while(node.first != -1 && node.second != -1){
            maze[node.first][node.second] = 'P';
            node = global_parent[node];
        }
    }
    maze[0][63] = 'S';
    maze[63][0] = 'E';
    // print the maze
    if(rank == 0){
        for(int i=0; i<2*MAZE_SIZE; i++){
            for(int j=0; j<2*MAZE_SIZE; j++){
                cout<<maze[i][j];
            }
            if(i != 2*MAZE_SIZE - 1){
                cout<<'\n';
            }
        }
    }
    return;
}


// This is a function to recursively solve the maze using DFS, for a given source and destination 
// This code is sequential but runs parallely on every processor
// thereby reducing the time by a factor of 4
// We use this code as a black box in our implementations
// Given a maze this code will solve and inplace alter the maze to show the path
// We then in mazesolver use all parallely computed paths to solve the maze 
// by using the gathered information from all processors

bool recursive_dfs(vector<vector<char>> &maze, vector<vector<int>> &visited, int i, int j, int rank) {
    if (i < 0 || i >= MAZE_SIZE/2 || j < 0 || j >= 2 * MAZE_SIZE || visited[i][j] == 1 || maze[i][j] == '*') {
        return false;
    }
    visited[i][j] = 1;
    if(i == dest_row && j == dest_col){
        maze[i][j] = 'P';
        return true;
    }
    if(recursive_dfs(maze, visited, i - 1, j, rank)){
        maze[i][j] = 'P';
        return true;
    }
    if(recursive_dfs(maze, visited, i + 1, j, rank)){
        maze[i][j] = 'P';
        return true;
    }
    if(recursive_dfs(maze, visited, i, j - 1, rank)){
        maze[i][j] = 'P';
        return true;
    }
    if(recursive_dfs(maze, visited, i, j + 1, rank)){
        maze[i][j] = 'P';
        return true;
    }
    return false;
}

// Every processor runs this dfs function on its local maze which is bounded from rbegin
// to rend. This function returns the local solved maze for the processor
// This function is called in mazesolver.cpp which then uses the gathered information
// to solve the maze parallely.
void dfs(vector<vector<char>> &maze, int rank, int comm_sz) {
    int rbegin = rank *2* MAZE_SIZE / comm_sz;
    int rend = min((rank + 1) * 2*MAZE_SIZE / comm_sz - 1, 2*MAZE_SIZE - 1);
    vector<vector<char>> local_solved_maze(MAZE_SIZE/2, vector<char>(2 * MAZE_SIZE)); 
    for(int r=rbegin; r<=rend; r++){
        for(int c=0; c<2*MAZE_SIZE; c++){
            local_solved_maze[r-rbegin][c] = maze[r][c];
        }
    }
    vector<vector<int>> visited(MAZE_SIZE/2, vector<int>(2 * MAZE_SIZE, 0));
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
    bool _ = recursive_dfs(local_solved_maze, visited, 0, c, rank);
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
    return;
}
