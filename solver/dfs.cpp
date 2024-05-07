#include "dfs.hpp"
#define MAZE_SIZE 32
using namespace std;

int dest_row, dest_col;

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

vector<vector<char>> dfs(vector<vector<char>> &maze, int rank, int comm_sz) {
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
    return local_solved_maze;
}
