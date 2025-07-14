#include "GridData.h"
#include <queue>

Vector2 gridToWorldPos(std::shared_ptr<GridData> g, Vector2 gridPos) {
    return Vector2{
        gridPos.x * g->tileWidth + (g->tileWidth / 2.0f),
        gridPos.y * g->tileHeight + (g->tileHeight / 2.0f)
    };
}

Vector2 worldToGridPos(std::shared_ptr<GridData> g, Vector2 worldPos) {
    return Vector2 (
        static_cast<int>(floor(worldPos.x / g->tileWidth)),
        static_cast<int>(floor(worldPos.y / g->tileHeight))
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
bool isValidPos(int* subcellArr, int w, int x, int y, int s) {
    //The x and y values represent the coords of the top-left corner of the agent. 
    //If this, plus the agent's size in subcells is outside the dimensions of the subcells grid,
    //return false
    if (x + s > w || y + s > w) //All tiles are square
        return false;
    //Checking if any of the subcells in the s*s grid with top-left at (x,y) are filled
    for (int i = x; i < x + s; i++) {
        for (int j = y; j < y + s; j++) {
            if (subcellArr[(i * w) + j] == 1)
                return false;
        }
    }
    return true;
}

//Returns an array of positions where an agent of size s*s can stand in a subcell grid
bool* preprocessValidPositions(int* subcellArr, int w, int s) {
    bool* retArr = (bool*)malloc(w*w* sizeof(bool));//The array being returned
    if (!retArr) return NULL;
    memset(retArr, false, w * w * sizeof(bool));//Initially assume all positions are impassible
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < w; j++) {
            //If this position is passable by the agent, can set it to true
            if (isValidPos(subcellArr, w, i, j, s))
                retArr[(i * w) + j] = true;
        }
    }
    return retArr;
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
bool pathExists(int startX, int startY, int s, int w, int*subcellArr, bool* pArr, Direction8 startDirection) {
    //If the start is invalid, return false
    if (!isValidPos(subcellArr, w, startX, startY, s)) return false;

    //Initialise the variables
    bool found = false;
    bool* visitedArr = (bool*)malloc(w * w * sizeof(bool)); //Marks which nodes we have visited
    if (!visitedArr) return false;
    memset(visitedArr, false, w * w * sizeof(bool)); //All nodes are initially unvisited
    std::queue<std::pair<int, int>> validPositions; //Queue to hold all of the positions to visit in bfs.
    //For iterating over neighbour coordinates
    int directionX[] = { -1, 1, 0, 0 };
    int directionY[] = { 0, 0, -1, 1 };

    //Add it to the queue of positions to run bfs on.
    validPositions.push({ startX, startY });
    visitedArr[(startY * w) + startX] = true; //Mark is as visited

    while (!validPositions.empty()) {
        std::pair<int, int> p = validPositions.front(); //Get first elem of queue

        //If this node is touching an edge, and that edge is not the start edge, then we have found a valid path 
        //out of the subcell array
        if (touchesEdge(p.first, p.second, s, w) && !checkEdge(p.first, p.second, s, w, startDirection)) {
            found = true;
            break;
        }

        //Visiting all of the cardinal neighbours of this cell
        for (int i = 0; i < 4; i++) {
            int newX = p.first + directionX[i];
            int newY = p.second + directionY[i];

            //Checking bounds are fine
            if (newX >= 0 && newX < w && newY >= 0 && newY < w && !visitedArr[(newY * w) + newX]) {
                visitedArr[(newY * w) + newX] = true; //Marking it as visited
                //Checking that it is a valid position, and pushing to the queue if it is
                if (pArr[(newY*w)+newX]) {
                    validPositions.push({ newX, newY });
                }
            }
        }
        //Removing the first element of the queue
        validPositions.pop();
    }
    free(visitedArr);

    return found; //No path out of the subcell grid found
}

//Gets the start position for an agent of size s*s in a subcell grid of size w*w
std::pair<int, int> getStartPos(int* subcellArr, int w, int s, Direction8 d) {
    switch (d) {
    case NW:
        if (isValidPos(subcellArr, w, 0, 0, s)) return std::make_pair(0, 0);
        return std::make_pair(-1, -1);
    case N:
        for (int i = 0; i < w - s; i++) {
            if (isValidPos(subcellArr, w, i, 0, s)) return std::make_pair(i, 0);
        }
        return std::make_pair(-1, -1);
    case NE:
        if (isValidPos(subcellArr, w, 0, w - s, s)) return std::make_pair(0, w - s);
        return std::make_pair(-1, -1);
    case E:
        for (int y = 0; y <= w - s; y++) {
            if (isValidPos(subcellArr, w, w - s, y, s)) return std::make_pair(w - s, y);
        }
        return std::make_pair(-1, -1);
    case SE:
        if (isValidPos(subcellArr, w, w - s, w - s, s)) return std::make_pair(w - s, w - s);
        return std::make_pair(-1, -1);
    case S:
        for (int x = 0; x <= w - s; x++) {
            if (isValidPos(subcellArr, w, x, w - s, s)) return std::make_pair(x, w - s);
        }
        return std::make_pair(-1, -1);
    case SW:
        if (isValidPos(subcellArr, w, 0, w - s, s)) return std::make_pair(0, w - s);
        return std::make_pair(-1, -1);
    case W:
        for (int y = 0; y <= w - s; y++) {
            if (isValidPos(subcellArr, w, 0, y, s)) return std::make_pair(0, y);
        }
        return std::make_pair(-1, -1);
    }
}

//Checks if there is a valid path to pass through the subcell grid
//if coming from a certain direction
bool isPathable(const TileData& t, Direction8 d, int s, int w) {
    //If there are not more than 2 sides that are exitable, then you can't really go through this partial tile
    if (numExits(t) < 2)
        return false;

    std::pair<int, int> startPos = getStartPos(t.subcells, w, s, d); //Get the start position

    if (startPos == std::make_pair(-1, -1)) return false; //Check that it is valid

    bool* prepArray = preprocessValidPositions(t.subcells, w, s);//Get the valid positions in the array
    if (!prepArray) return false;

    bool ret = pathExists(startPos.first, startPos.second, s, w, t.subcells, prepArray, d); //Check that a valid path exists through the tile

    free(prepArray); //Prevent memory leak;

    return ret;
}

//For combining tiles along the four cardinal directions
int* combineTiles(std::vector<int*> tArray, int w) {
    int* retArray = (int*)malloc(w * w * tArray.size() *sizeof(int));
    if (!retArray) return NULL;
    memset(retArray, 0, w * w * tArray.size() * sizeof(int));

    for(int i = 0; i < tArray.size(); i++) {
        for(int j = 0; j < w*w; j++) {
                retArray[(i*w*w)+j] = tArray[i][j];
        }
    }
    return retArray;
}

int* getCombinedSubcellGrid(int index, std::shared_ptr<GridData> g, Direction8 d) {
    std::vector<int*> tArr; //The returned vector
    //The temp array that represents the player position. Only used for diagonal traversals
    int* tempArr = (int*)malloc(g->subWidth * g->subWidth * sizeof(int));
    if (!tempArr) return NULL;
    memset(tempArr, 0, g->subWidth * g->subWidth * sizeof(int));

    //Getting the order in which we submit the subcell data to be combined together
    switch (d) {
        case N:
        case S:
            if (index < 0 || index >= g->tiles.size()) {
                free(tempArr);
                return NULL;
            }
            else if (index % g->gridWidth == 0) tArr = { g->tiles[index].subcells, g->tiles[index + 1].subcells };
            else if (index % g->gridWidth == g->gridWidth - 1) tArr = { g->tiles[index - 1].subcells, g->tiles[index].subcells };
            else tArr = { g->tiles[index - 1].subcells, g->tiles[index].subcells, g->tiles[index + 1].subcells };
            break;
        case E:
        case W:
            if (index < 0 || index >= g->tiles.size()) {
                free(tempArr);
                return NULL;
            }
            else if (index / g->gridWidth == 0) tArr = { g->tiles[index].subcells, g->tiles[index + g->gridWidth].subcells };
            else if (index % g->gridWidth == g->gridHeight - 1) tArr = { g->tiles[index - g->gridWidth].subcells, g->tiles[index].subcells };
            else tArr = { g->tiles[index - g->gridWidth].subcells, g->tiles[index].subcells, g->tiles[index + g->gridWidth].subcells };
            break;
        case NW:
            if (index < 0 || index >= g->tiles.size()) {
                free(tempArr);
                return NULL;
            }
            tArr = {g->tiles[index].subcells, g->tiles[index+1].subcells, g->tiles[index - g->gridWidth].subcells, tempArr};
            break;
        case NE:
            if (index < 0 || index >= g->tiles.size()) {
                free(tempArr);
                return NULL;
            }
            tArr = { g->tiles[index].subcells, g->tiles[index + 1].subcells, tempArr, g->tiles[index - g->gridWidth].subcells };
            break;
        case SW:
            if (index < 0 || index >= g->tiles.size()) {
                free(tempArr);
                return NULL;
            }
            tArr = { g->tiles[index - g->gridWidth].subcells, tempArr, g->tiles[index].subcells, g->tiles[index + 1].subcells };
            break;
        case SE:
            if (index < 0 || index >= g->tiles.size()) {
                free(tempArr);
                return NULL;
            }
            tArr = { tempArr, g->tiles[index - g->gridWidth].subcells, g->tiles[index].subcells, g->tiles[index + 1].subcells };
            break;
    }
    int* retArr = combineTiles(tArr, g->subWidth);
    free(tempArr); //Freeing the temporary array
    return retArr;
}

//For processing pathfinding across adjacent tiles;
bool isPathableWithAdjacent(int index, std::shared_ptr<GridData> g, Direction8 d, int s) {
    if (numExits(g->tiles[index]) < 2) return false;

    //Determining the correct width based on orientation
    int width;
    if (d == N || d == S)
        width = 3;
    else if (d == W || d == E)
        width = 1;
    else
        width = 2;
    width *= g->subWidth;

    std::pair<int, int> startPos = getStartPos(g->tiles[index].subcells, g->subWidth, s, d); //Get the start position

    if (startPos == std::make_pair(-1, -1)) return false; //Check that it is valid

    //Need to adjust start position to work in the overall combined grid
    if (d == N || d == S)
        startPos.first += g->subWidth;
    else if (d == W || d == E)
        startPos.second += g->subWidth;
    else {
        startPos.first = g->subWidth;
        startPos.second = g->subWidth;
    }

    int* combinedCells = getCombinedSubcellGrid(index, g, d); //Get the combined subcell grid
    if (!combinedCells) return false;

    bool* prepArray = preprocessValidPositions(combinedCells, width, s);//Get the valid positions in the array
    if (!prepArray) {
        free(combinedCells);
        return false;
    }

    bool ret = pathExists(startPos.first, startPos.second, s, width, combinedCells, prepArray, d); //Check that a valid path exists through the tile

    //Prevent memory leaks
    free(combinedCells);
    free(prepArray); 

    return ret;
}