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
bool isValidPos(std::vector<int> subcellArr, int w, int x, int y, int s) {
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
std::vector<bool> preprocessValidPositions(std::vector<int> subcellArr, int w, int s) {
    std::vector<bool> retArr(w*w, false);//The array being returned. Initially assume all positions are impassible
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

//Whether the agent standing at position at (x,y) would exit to the same direction 
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
bool pathExists(int startX, int startY, int s, int w, std::vector<bool> pArr, Direction8 startDirection) {
    //If the start is invalid, return false
    //if (!pArr[(startY*w)+startX]) return false;

    //Initialise the variables
    bool found = false;
    bool* visitedArr = (bool*)malloc(w * w * sizeof(bool)); //Marks which nodes we have visited
    if (!visitedArr) return false;
    memset(visitedArr, false, w * w * sizeof(bool)); //All nodes are initially unvisited
    std::queue<std::pair<int, int>> validPositions; //Queue to hold all of the positions to visit in bfs.
    //For iterating over neighbour coordinates.
    //We can only allow diagonal traversal between valid tiles when agents are of size > 1, since the 
    //issue of agents crashing into walls forever should be avoided by the fact that valid tiles 
    //should be surrounded by other tiles which are technically passable, but not valid, so this should
    //allow for diagonal traversal
    std::vector<int> directionX = (s > 1) ? std::vector<int>{-1, 0, 1, 0, 1, 1, -1, -1} : std::vector<int>{ -1, 1, 0, 0 };
    std::vector<int> directionY = (s > 1) ? std::vector<int>{0, 1, 0, -1, 1, -1, 1, -1} : std::vector<int>{ 0, 0, -1, 1 };

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
        for (int i = 0; i < directionX.size(); i++) {
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

bool pathExistsTo(int startX, int startY, int endX, int endY, int s, int w, std::vector<bool> pArr) {
    //If the start is invalid, return false
    //if (!pArr[(startY*w)+startX] ||!pArr[(endY*w)+endX]) return false;

    //Initialise the variables
    bool found = false;
    bool* visitedArr = (bool*)malloc(w * w * sizeof(bool)); //Marks which nodes we have visited
    if (!visitedArr) return false;
    memset(visitedArr, false, w * w * sizeof(bool)); //All nodes are initially unvisited
    std::queue<std::pair<int, int>> validPositions; //Queue to hold all of the positions to visit in bfs.
    //For iterating over neighbour coordinates
    //We can only allow diagonal traversal between valid tiles when agents are of size > 1, since the 
    //issue of agents crashing into walls forever should be avoided by the fact that valid tiles 
    //should be surrounded by other tiles which are technically passable, but not valid, so this should
    //allow for diagonal traversal
    std::vector<int> directionX = (s > 1) ? std::vector<int>{-1, 0, 1, 0, 1, 1, -1, -1} : std::vector<int>{ -1, 1, 0, 0 };
    std::vector<int> directionY = (s > 1) ? std::vector<int>{0, 1, 0, -1, 1, -1, 1, -1} : std::vector<int>{ 0, 0, -1, 1 };

    //Add it to the queue of positions to run bfs on.
    validPositions.push({ startX, startY });
    visitedArr[(startY * w) + startX] = true; //Mark is as visited

    while (!validPositions.empty()) {
        std::pair<int, int> p = validPositions.front(); //Get first elem of queue

        //If this node is touching an edge, and that edge is not the start edge, then we have found a valid path 
        //out of the subcell array
        if (p.first == endX && p.second == endY) {
            found = true;
            break;
        }

        //Visiting all of the cardinal neighbours of this cell
        for (int i = 0; i < directionX.size(); i++) {
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
std::pair<int, int> getStartPos(std::vector<int> subcellArr, int w, int s, Direction8 d) {
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

    std::vector<bool> prepArray = preprocessValidPositions(t.subcells, w, s);//Get the valid positions in the array

    bool ret = pathExists(startPos.first, startPos.second, s, w, prepArray, d); //Check that a valid path exists through the tile

    return ret;
}

//For combining tiles cardinal directions
std::vector<int> combineTiles(std::vector<std::vector<int>*> tArray, int w) {
    std::vector<int> retArray(w*w*tArray.size(), 0);

    for(int i = 0; i < tArray.size(); i++) {
        for(int j = 0; j < w*w; j++) {
                retArray[(i*w*w)+j] = tArray[i]->at(j);
        }
    }
    return retArray;
}

std::vector<int> getCombinedSubcellGrid(int index, std::shared_ptr<GridData> g, Direction8 d) {
    std::vector<std::vector<int>*> tArr; //The vector being passed to combine all the cells
    //The temp array that represents the player position. Only used for diagonal traversals
    std::vector<int> tempArr(g->subWidth*g->subWidth, 0);

    //Getting the order in which we submit the subcell data to be combined together
    switch (d) {
        case N:
        case S:
            if (index < 0 || index >= g->tiles.size()) {
                return std::vector<int>();
            }
            else if (index % g->gridWidth == 0) tArr = { &g->tiles[index].subcells, &g->tiles[index + 1].subcells };
            else if (index % g->gridWidth == g->gridWidth - 1) tArr = { &g->tiles[index - 1].subcells, &g->tiles[index].subcells };
            else tArr = { &g->tiles[index - 1].subcells, &g->tiles[index].subcells, &g->tiles[index + 1].subcells };
            break;
        case E:
        case W:
            if (index < 0 || index >= g->tiles.size()) {
                return std::vector<int>();
            }
            else if (index / g->gridWidth == 0) tArr = { &g->tiles[index].subcells, &g->tiles[index + g->gridWidth].subcells };
            else if (index / g->gridWidth == g->gridHeight - 1) tArr = { &g->tiles[index - g->gridWidth].subcells, &g->tiles[index].subcells };
            else tArr = { &g->tiles[index - g->gridWidth].subcells, &g->tiles[index].subcells, &g->tiles[index + g->gridWidth].subcells };
            break;
        case NW:
            if (index < 0 || index >= g->tiles.size()) {
                return std::vector<int>();
            }
            tArr = {&g->tiles[index].subcells, &g->tiles[index+1].subcells, &g->tiles[index - g->gridWidth].subcells, &tempArr};
            break;
        case NE:
            if (index < 0 || index >= g->tiles.size()) {
                return std::vector<int>();
            }
            tArr = { &g->tiles[index].subcells, &g->tiles[index + 1].subcells, &tempArr, &g->tiles[index - g->gridWidth].subcells };
            break;
        case SW:
            if (index < 0 || index >= g->tiles.size()) {
                return std::vector<int>();
            }
            tArr = { &g->tiles[index - g->gridWidth].subcells, &tempArr, &g->tiles[index].subcells, &g->tiles[index + 1].subcells };
            break;
        case SE:
            if (index < 0 || index >= g->tiles.size()) {
                return std::vector<int>();
            }
            tArr = { &tempArr, &g->tiles[index - g->gridWidth].subcells, &g->tiles[index].subcells, &g->tiles[index + 1].subcells };
            break;
    }
    return combineTiles(tArr, g->subWidth);
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
    
    //THIS ISN'T RIGHT!!!!!!!! WILL FAIL IF CANNOT FIT ONTO TILE AT ALL IN THIS DIRECTION
    //NEED TO CONSIDER ADJACENT TILES IN STARTPOS CALC TOO!!!!!
    //Maybe just set size to 1 to avoid problems for now
    std::pair<int, int> startPos = getStartPos(g->tiles[index].subcells, g->subWidth, 1, d); //Get the start position

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

    std::vector<int> combinedCells = getCombinedSubcellGrid(index, g, d); //Get the combined subcell grid

    std::vector<bool> prepArray = preprocessValidPositions(combinedCells, width, s);//Get the valid positions in the array

    return pathExists(startPos.first, startPos.second, s, width, prepArray, d); //Check that a valid path exists through the tile
}

//A bunch of helper bools for the trimCells function
bool isAtTopEdge(Vector2 vec) {
    return vec.y == -1;
}

bool isAtLeftEdge(Vector2 vec) {
    return vec.x == -1;
}

bool isAtRightEdge(Vector2 vec) {
    return vec.x == 1;
}

bool isAtBottomEdge(Vector2 vec) {
    return vec.y == 1;
}

//Returns a vector of Vector2s that represent all the coordinates of the 
//"valid" neighbour cells of the current index
//It removes all the non-valid neighbours (those that would end up outside the grid)
//from the vector it returns
std::vector<Vector2> trimCells(int index, int gridWidth, int gridHeight, Direction8 d) {
    //Break check on bad indeces
    if(index < 0 || index >= (gridWidth * gridHeight)) {
        return std::vector<Vector2>();
    }

    //The basic vector contains all of the coordinates of the neighbour cells, as well as the current cell
    std::vector<Vector2> retVec = {Vector2(-1, -1), Vector2(0, -1), Vector2(1, -1), 
                                   Vector2(-1,  0), Vector2(0, 0),  Vector2(1, 0), 
                                   Vector2(-1, 1),  Vector2(0, 1),  Vector2(1, 1)};
    
    //Trimming the cells based on direction
    switch(d) {
        case N:
            retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtBottomEdge), retVec.end());
            break;
        case E:
            retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtLeftEdge), retVec.end());
            break;
        case S:
            retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtTopEdge), retVec.end());
            break;
        case W:
            retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtRightEdge), retVec.end());
            break;
        case NE:
            retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtBottomEdge), retVec.end());
            retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtLeftEdge), retVec.end());
            break;
        case NW:
            retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtBottomEdge), retVec.end());
            retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtRightEdge), retVec.end());
            break;
        case SE:
            retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtTopEdge), retVec.end());
            retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtLeftEdge), retVec.end());
            break;
        case SW:
            retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtTopEdge), retVec.end());
            retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtRightEdge), retVec.end());
            break;
    }
    
    //Trimming the cells if we are at an edge
    if(index % gridWidth == 0) {
        retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtLeftEdge), retVec.end());
    } 
    if(index % gridWidth == gridWidth - 1){
        retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtRightEdge), retVec.end());
    }
    if(index / gridWidth == 0) {
        retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtTopEdge), retVec.end());
    }
    if(index / gridWidth == gridHeight - 1) {
        retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtBottomEdge), retVec.end());
    }

    return retVec;
}

std::vector<int>* getNeighbourCells(int index, std::shared_ptr<GridData>g, Vector2 vec) {
    return &g->tiles[index+ (g->gridWidth*vec.y) + vec.x].subcells;
}

std::vector<int> createSurroundGrid(int index, std::shared_ptr<GridData> g, Direction8 d) {
    //Get the coordinates of all of the neighbours of the cell that are inside the grid
    std::vector<Vector2> neighbours = trimCells(index, g->gridWidth, g->gridHeight, d);

    //Create the new vector:
    std::vector<std::vector<int>*> subcellArr(neighbours.size());
    std::transform(neighbours.begin(), neighbours.end(), subcellArr.begin(),
                    [index, g](Vector2 v) {return getNeighbourCells(index, g, v);});

    //Combine the neighbour and current cells together
    return combineTiles(subcellArr, g->subWidth);
}

bool isPathBetween(Direction8 from, Direction8 to, std::shared_ptr<GridData> g, int indexAt, int indexTo, int s) {
    //Make the grid subset that we need to path on to get to the desired direction
    std::vector<int> combinedCells = createSurroundGrid(indexAt, g, to);

    //The width of the combined grid
    int width = (indexAt % g->gridWidth == 0 || indexAt % g->gridWidth == g->gridWidth-1) ? 2 : 3;
    width *= g->subWidth;

    //Find startPosition and endPosition depending on from and to
    std::pair<int, int> startPos = getStartPos(g->tiles[indexAt].subcells, g->subWidth, s, from);
    if (startPos == std::make_pair(-1, -1)) return false; //Check that it is valid
    std::pair<int, int> endPos = getStartPos(g->tiles[indexTo].subcells, width, s, to);
    if (endPos == std::make_pair(-1, -1)) return false; //Check that it is valid

    //Need to adjust start position to work in the overall combined grid
    /*if (from == N || from == S)
        startPos.first += g->subWidth;
    else if (from == W || from == E)
        startPos.second += g->subWidth;
    else {
        startPos.first = g->subWidth;
        startPos.second = g->subWidth;
    }*/
    //I think these are the correct adjusted start and end positions
    switch(from) {
        case N:
        case S:
            startPos.first += g->subWidth;
            break;
        case E:
        case W:
            startPos.second += g->subWidth;
            break;
        case NW:
            startPos.first = 0;
            startPos.second = 0;
            break;
        case NE:
            startPos.first = g->subWidth-1;
            startPos.second = 0;
            break;
        case SW:
            startPos.first = 0;
            startPos.second = g->subWidth-1;
            break;
        case SE:
            endPos.first = g->subWidth;
            endPos.second = g->subWidth;
            break;
    }
 
    switch(to) {
        case N:
            endPos.second -= g->subWidth;
            endPos.first += g->subWidth;
            break;
        case S:
            endPos.second += g->subWidth;
            endPos.first += g->subWidth;
            break;
        case E:
            endPos.first += g->subWidth;
            endPos.second += g->subWidth;
            break;
        case W:
            endPos.first -= g->subWidth;
            endPos.second += g->subWidth;
            break;
        default:
            endPos.second = g->subWidth;
            endPos.first = g->subWidth;
            break;
    }
    //Find if there is a path between them 
    std::vector<bool> prepArray = preprocessValidPositions(combinedCells, width, s);//Get the valid positions in the array

    return pathExistsTo(startPos.first, startPos.second, endPos.first, endPos.second, s, width, prepArray); //Check that a valid path exists through the tile
}
