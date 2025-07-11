#include "GridData.h"
#include <queue>

Vector2 gridToWorldPos(std::shared_ptr<GridData> g, Vector2 gridPos) {
    return Vector2{
        gridPos.x * g->tileWidth + (g->tileWidth / 2.0f),
        gridPos.x * g->tileHeight + (g->tileHeight / 2.0f)
    };
}

Vector2 worldToGridPos(std::shared_ptr<GridData> g, Vector2 worldPos) {
    return Vector2 (
        static_cast<int>(floor(worldPos.x / (g->tileWidth / 2))),
        static_cast<int>(floor(worldPos.y / (g->tileHeight / 2)))
    );
}

int toIndex(std::shared_ptr<GridData> g, Vector2 gridPos) {
    return gridPos.x + gridPos.y * g->gridWidth;
}
bool inBounds(std::shared_ptr<GridData> g, Vector2 gridPos) {
    return gridPos.x >= 0 && gridPos.y >= 0 && gridPos.x < g->gridWidth && gridPos.y < g->gridHeight;
}

//Gets the number of exitable sides a subcell grid has
int numExits(const TileData& t) {
    //Simple for loop
    int count = 0; //Counter
    for (int i = 0; i < 4; i++) {
        //The TileData struct stores which of its exits are "exitable".
        //This just means they have some passable tiles
        if (t.exitable[i])
            count++;
    }
    return count;
}

//If an agent of dimensions s*s can stand in a position (x,y) in a subcell grid
bool isValidPos(const TileData& t, int x, int y, int s) {
    //The x and y values represent the coords of the top-left corner of the agent. 
    //If this, plus the agent's size in subcells is outside the dimensions of the subcells grid,
    //return false
    if (x + s > 4 || y + s > 4)
        return false;
    //Checking if any of the subcells in the s*s grid with top-left at (x,y) are filled
    for (int i = x; i < x + s; i++) {
        for (int j = x; j < y + s; j++) {
            if (t.subcells[(i * 4) + j] == 1)
                return false;
        }
    }
    return true;
}

//Returns an array of positions where an agent of size s*s can stand in a subcell grid
std::array<bool, 16> preprocessValidPositions(const TileData& t, int s) {
    std::array<bool, 16> retArray;//The array being returned
    retArray.fill(false); //Initially assume all positions are impassible
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            //If this position is passable by the agent, can set it to true
            if (isValidPos(t, i, j, s))
                retArray[(i * 4) + j] = true;
        }
    }
    return retArray;
}

//Whether an agent of size s standing at position (x, y) touches the edge of a subcell grid
bool touchesEdge(int x, int y, int s, int n) {
    return x == 0 || y == 0 || x + s == n || y + s == n;
}

//Whether the agent standing at position at (x,y) would exit to a different direction 
//to the one it entered the subcell grid from
bool checkEdge(int x, int y, int s, int n, Direction8 d) {
    switch (d) {
        //Cardinal directions only need one coordinate checked
        case N:
            if (y == 0) return true;
            return false;
        case E:
            if (x + s == n) return true;
            return false;
        case S:
            if (y + s == n) return true;
            return false;
        case W:
            if (x == 0) return true;
            return false;
        //Diagonal directions need both coordinates checked
        case NW:
            if (y == 0 && x == 0) return true;
            return false;
        case NE:
            if (y == 0 && x + s == n) return true;
            return false;
        case SW:
            if (y + s == n && x == 0) return true;
            return false;
        case SE:
            if (y + s == n && x + s == n) return true;
            return false;
    }
}

//Runs a version of bfs to check if a path exists from a specific starting position for an agent of size s*s 
//to pass through
bool pathExists(int startX, int startY, int s, const TileData& t, const std::array<bool, 16>& pArr, Direction8 startDirection) {
    std::array<bool, 16> visitedArr; //Marks which nodes we have visited
    visitedArr.fill(false); //All nodes are initially unvisited
    std::queue<std::pair<int, int>> validPositions; //Queue to hold all of the positions to visit in bfs.
    //For iterating over neighbour coordinates
    int directionX[] = { -1, 1, 0, 0 };
    int directionY[] = { 0, 0, -1, 1 };

    //If the start is invalid, return false
    if (!isValidPos(t, startX, startY, s)) return false;

    //Add it to the queue of positions to run bfs on.
    validPositions.push({ startX, startY });
    visitedArr[(startY * 4) + startX] = true; //Mark is as visited

    while (!validPositions.empty()) {
        std::pair<int, int> p = validPositions.front(); //Get first elem of queue
        
        //If this node is touching an edge, and that edge is not the start edge, then we have found a valid path 
        //out of the subcell array
        if (touchesEdge(p.first, p.second, s, 4) && !checkEdge(p.first, p.second, s, 4, startDirection)) return true;

        //Visiting all of the cardinal neighbours of this cell
        for (int i = 0; i < 4; i++) {
            int newX = p.first + directionX[i];
            int newY = p.second + directionY[i];

            //Checking bounds are fine
            if (newX >= 0 && newX < 4 && newY >= 0 && newY < 4 && !visitedArr[(newY * 4) + newX]) {
                visitedArr[(newY * 4) + newX] = true; //Marking it as visited
                //Checking that it is a valid position, and pushing to the queue if it is
                if (isValidPos(t, newX, newY, s)) {
                    validPositions.push({ newX, newY });
                }
            }
        }
        //Removing the first element of the queue
        validPositions.pop();
    }
    return false; //No path out of the subcell grid found
}

//Gets a valid start position for an agent of size s*s coming from a specific direction as a pair
//returning (-1,-1) if no valid start position was found
std::pair<int, int> getStartPos(const TileData& t, int s, Direction8 d) {
    switch (d) {
        case NW:
            if (isValidPos(t, 0, 0, s)) return std::make_pair(0, 0);
            return std::make_pair(-1, -1);
        case N:
            for (int i = 0; i < 4-s; i++) {
                if (isValidPos(t, i, 0, s)) return std::make_pair(i, 0);
            }
            return std::make_pair(-1, -1);
        case NE:
            if (isValidPos(t, 0, 4-s, s)) return std::make_pair(0, 4-s);
            return std::make_pair(-1, -1);
        case E:
            for (int i = 3; i < 16 - (s*4); i+=4) {
                if (isValidPos(t, 4-s, i, s)) return std::make_pair(4-s, i);
            }
            return std::make_pair(-1, -1);
        case SE:
            if (isValidPos(t, 4-s, 4-s, s)) return std::make_pair(4-s, 4-s);
            return std::make_pair(-1, -1);
        case S:
            for (int i = 12; i < 16 - s; i++) {
                if (isValidPos(t, i, 4-s, s)) return std::make_pair(i, 4-s);
            }
            return std::make_pair(-1, -1);
        case SW:
            if (isValidPos(t, 0, 4-s, s)) return std::make_pair(0, 4-s);
            return std::make_pair(-1, -1);
        case W:
            for (int i = 0; i < 12 - (s*4); i+=4) {
                if (isValidPos(t, 0, i, s)) return std::make_pair(0, i);
            }
            return std::make_pair(-1, -1);
    }
}

//Checks if there is a valid path to pass through the subcell grid
//if coming from a certain direction
bool isPathable(const TileData& t, Direction8 d, int s) {
    //If there are not more than 2 sides that are exitable, then you can't really go through this partial tile
    if (numExits(t) < 2)
        return false;

    std::array<bool, 16> prepArray = preprocessValidPositions(t, s);//Get the valid positions in the array

    std::pair<int, int> startPos = getStartPos(t, s, d); //Get the start position
    
    if (startPos == std::make_pair( - 1, -1 )) return false; //Check that it is valid

    return pathExists(startPos.first, startPos.second, s, t, prepArray, d); //Check that a valid path exists through the tile
}