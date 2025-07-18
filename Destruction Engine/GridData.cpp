#include "GridData.h"
#include <queue>
#include <iostream>

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
    for (int i = x; i < x + s; i++) {//x position
        for (int j = y; j < y + s; j++) {//y position
            if (subcellArr[(j * w) + i] == 1)
                return false;
        }
    }
    return true;
}

//Returns an array of positions where an agent of size s*s can stand in a subcell grid
std::vector<bool> preprocessValidPositions(std::vector<int> subcellArr, int w, int s) {
    std::vector<bool> retArr(w*w, false);//The array being returned. Initially assume all positions are impassible
    for (int i = 0; i < w; i++) {//x position
        for (int j = 0; j < w; j++) {//y position
            //If this position is passable by the agent, can set it to true
            if (isValidPos(subcellArr, w, i, j, s))
                retArr[(j * w) + i] = true;
        }
    }
    return retArr;
}

//Whether an agent of size s standing at position (x, y) touches the edge of a subcell grid
bool touchesEdge(int x, int y, int s, int n) {
    return /*x == 0 || y == 0 || x + s == n || y + s == n || */
    (x <= 0 && x+s >= 0) || (y <= 0 && x+s >= 0) || (x < n && x+ s>= n) || (y < n && y + s >=n);
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

Direction8 getStartDirectionByPosition(int x, int y, int w){
    if((x > 0 && x != w - 1) && ( y > 0 && y != w - 1)) return INVALID; 
    
    if(x == 0 && y == 0) return NW;
    if(x == 0 && (y > 0 && y < w - 1)) return W;
    if(x == 0 && y == w - 1) return SW;
    if(x > 0 && x < w - 1 && y == w - 1) return S;
    if(x == w - 1 && y == w - 1) return SE;
    if(x == w - 1 && y > 0 && y < w - 1) return E;
    if(x == w - 1 && y == 0) return NE;
    if(x > 0 && x < w - 1 && y == 0) return N;

    return INVALID; //Some other bug
}

//Runs a version of bfs to check if a path exists from a specific starting position for an agent of size s*s 
//to pass through
//startX, startY are the starting x and y positions.
//s is the size of the agent
//w is the width of the subcell array we are working on
//pArr is the array of preprocessed valid positions
//startDirection is the direction we entered the tile from
bool pathExists(int startX, int startY, int s, int w, std::vector<bool> pArr, Direction8 startDirection) {
    //If the start is invalid, return false
    //if (!pArr[(startY*w)+startX]) return false;

    //Initialise the variables
    // Direction8 startDirection = getStartDirectionByPosition(startX, startY, w);
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

//startX, startY are the starting x and y positions.
//endX, endY are the ending x and y positions.s
//s is the size of the agent
//w is the width of the subcell array we are working on
//pArr is the array of preprocessed valid positions
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
        if (p.first + s >= endX && p.second + s >= endY) {
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
//subcellArr is the subcell array we are working with
//w is the width of the array
//s is the size of the agent
std::pair<int, int> getStartPos(std::vector<int> subcellArr, int w, int h, int s, Direction8 d) {
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
        if (isValidPos(subcellArr, w, w - s, 0, s)) return std::make_pair(w - s, 0);
        return std::make_pair(-1, -1);
    case E:
        for (int y = 0; y <= h - s; y++) {
            if (isValidPos(subcellArr, w, w - s, y, s)) return std::make_pair(w - s, y);
        }
        return std::make_pair(-1, -1);
    case SE:
        if (isValidPos(subcellArr, w, w - s, h - s, s)) return std::make_pair(w - s, h - s);
        return std::make_pair(-1, -1);
    case S:
        for (int x = 0; x <= w - s; x++) {
            if (isValidPos(subcellArr, w, x, h - s, s)) return std::make_pair(x, h - s);
        }
        return std::make_pair(-1, -1);
    case SW:
        if (isValidPos(subcellArr, w, 0, h - s, s)) return std::make_pair(0, h - s);
        return std::make_pair(-1, -1);
    case W:
        for (int y = 0; y <= h - s; y++) {
            if (isValidPos(subcellArr, w, 0, y, s)) return std::make_pair(0, y);
        }
        return std::make_pair(-1, -1);
    }
}

Direction8 getOppositeDirection(Direction8 d) {
    switch(d) {
        case N: return S;
        case E: return W;
        case S: return N;
        case W: return E;
        case NW: return SE;
        case NE: return SW;
        case SE: return NW;
        case SW: return NE;
    }
}

//Checks if there is a valid path to pass through the subcell grid
//if coming from a certain direction
bool isPathable(const TileData& t, Direction8 d, int s, int w) {
    //If there are not more than 2 sides that are exitable, then you can't really go through this partial tile
    if (numExits(t) < 2)
        return false;

    std::pair<int, int> startPos = getStartPos(t.subcells, w, w, s, getOppositeDirection(d)); //Get the start position

    if (startPos == std::make_pair(-1, -1)) return false; //Check that it is valid

    std::vector<bool> prepArray = preprocessValidPositions(t.subcells, w, s);//Get the valid positions in the array

    bool ret = pathExists(startPos.first, startPos.second, s, w, prepArray, getOppositeDirection(d)); //Check that a valid path exists through the tile

    return ret;
}

std::vector<int> combineTiles(std::vector<std::vector<int>*> tArray, int tileW, int newGridW, int newGridH) {
    std::vector<int> retArray(tileW*tileW*tArray.size(), 0);

    //This is incredibly disgusting, and unfortunately very necessary 
    for(int gridX = 0; gridX < newGridW; gridX++) {
        for(int gridY = 0; gridY < newGridH; gridY++) {
            for(int tileX = 0; tileX < tileW; tileX++) {
                for(int tileY = 0; tileY < tileW; tileY++) {
                    int combinedWidth = tileW * newGridW;
                    int dstX = gridX * tileW + tileX;
                    int dstY = gridY * tileW + tileY;
                    retArray[dstY * combinedWidth + dstX] = 
                    tArray[(gridY*newGridW) + gridX]->at((tileY*tileW) + tileX);
                }
            }
        }
    }
    return retArray;
}

//Gets the combined subcell grid of the cells that need to be passed through
//Index is the index of the starting cell
//g is the GridData representing the grid used for navigation
//d is the direction that we need to be moving in
std::vector<int> getCombinedSubcellGrid(int index, std::shared_ptr<GridData> g, Direction8 d) {
    std::vector<std::vector<int>*> tArr; //The vector being passed to combine all the cells
    //The temp array that represents the player position. Only used for diagonal traversals
    std::vector<int> tempArr(g->subWidth*g->subWidth, 0);

    //Out of bounds check
    if (index < 0 || index >= g->tiles.size()) return std::vector<int>();

    //Getting the order in which we submit the subcell data to be combined together
    switch (d) {
        case N:
        case S:
            //If we are on the northern/southern edges, then we cannot move north/south
            if(d == N && index / g->subWidth <= 0) return std::vector<int>();
            if(d == S && index / g->subWidth >= g->subWidth - 1) return std::vector<int>();

            //Adjusting the starting index
            if(d == N) index -= g->subWidth;
            else index += g->subWidth;
            
            if (index < 0 || index >= g->tiles.size()) return std::vector<int>();
            else if (index % g->gridWidth == 0) {
                tArr = { &g->tiles[index].subcells, &g->tiles[index + 1].subcells };
                return combineTiles(tArr, g->subWidth, 2, 1);
            }
            else if (index % g->gridWidth == g->gridWidth - 1) {
                tArr = { &g->tiles[index - 1].subcells, &g->tiles[index].subcells }; 
                return combineTiles(tArr, g->subWidth, 2, 1); 
            } 
            else {
                tArr = { &g->tiles[index - 1].subcells, &g->tiles[index].subcells, &g->tiles[index + 1].subcells };
                return combineTiles(tArr, g->subWidth, 3, 1);
            }
        case E:
        case W:
            //If we are on the western/easter edges, then we cannot move west/east
            if(d == W && index % g->subWidth <= 0) return std::vector<int>();
            if(d == E && index % g->subWidth >= g->subWidth - 1) return std::vector<int>();

            if(d == E) index += 1;
            else index -= 1;

            if (index < 0 || index >= g->tiles.size()) return std::vector<int>();
            else if (index / g->gridWidth == 0) {
                tArr = { &g->tiles[index].subcells, &g->tiles[index + g->gridWidth].subcells };
                return combineTiles(tArr, g->subWidth, 1, 2);
            }
            else if (index / g->gridWidth == g->gridHeight - 1) {
                tArr = { &g->tiles[index - g->gridWidth].subcells, &g->tiles[index].subcells };
                return combineTiles(tArr, g->subWidth, 1, 2);
            }
            else {
                tArr = { &g->tiles[index - g->gridWidth].subcells, &g->tiles[index].subcells, &g->tiles[index + g->gridWidth].subcells };
                return combineTiles(tArr, g->subWidth, 1, 3);
            }
            
        case NW:
            //If we are outside the bounds of the grid, or on the northern or western edges, then we cannot move northwest
            if (index < 0 || index >= g->tiles.size() || index % g->subWidth == 0 || index / g->subWidth == 0) {
                return std::vector<int>();
            }
            tArr = {&g->tiles[(index - g->gridWidth) - 1].subcells, &g->tiles[index - g->gridWidth].subcells, &g->tiles[index-1].subcells, &tempArr};
            return combineTiles(tArr, g->subWidth, 2, 2);
        case NE:
            //If we are outside the bounds of the grid, or on the northern or eastern edges, then we cannot move northeast
            if (index < 0 || index >= g->tiles.size() || index % g->subWidth == g->subWidth - 1 || index / g->subWidth == 0) {
                return std::vector<int>();
            }
            tArr = {&g->tiles[index - (g->gridWidth)].subcells, &g->tiles[(index - g->gridWidth) + 1].subcells, &tempArr, &g->tiles[index+1].subcells};
            return combineTiles(tArr, g->subWidth, 2, 2);
        case SW:
            //If we are outside the bounds of the grid, or on the southern or eastern edges, then we cannot move southwest
            if (index < 0 || index >= g->tiles.size() || index % g->subWidth == 0 || index / g->subWidth == g->subWidth - 1) {
                return std::vector<int>();
            }
            tArr = { &g->tiles[index - 1].subcells, &tempArr, &g->tiles[(index + g->subWidth) - 1].subcells, &g->tiles[index + g ->subWidth].subcells };
            return combineTiles(tArr, g->subWidth, 2, 2);
        case SE:
            //If we are outside the bounds of the grid, or on the sourthern or eastern edges, then we cannot move southeast
            if (index < 0 || index >= g->tiles.size() || index % g->subWidth == g->subWidth - 1 || index / g->subWidth == g->subWidth - 1) {
                return std::vector<int>();
            }
            tArr = { &tempArr, &g->tiles[index +1].subcells, &g->tiles[index + g->subWidth].subcells, &g->tiles[index + g->subWidth + 1].subcells };
            return combineTiles(tArr, g->subWidth, 2, 2);
    }
    //return combineTiles(tArr, g->subWidth);
}

int getWidth(int index, int w, Direction8 d) {
    int width = 0;
    if ((d == N || d == S) && (index % w != 0 && index % w != (w - 1)))
        width = 3;
    else if ((d == W || d == E) && (index / w != 0 && index / w != (w - 1)))
        width = 1;
    else
        width = 2;
    width *= w;
    return width;
}

std::pair<int, int> getDimensions(int index, int w, Direction8 d) {
    int width = 0;
    int height = 0;

    if (d == N || d == S){
        if (index % w != 0 && index % w != (w - 1)) {
            width = 3;
            height = 1;
        }
        else {
            width = 2;
            height = 1;
        }
    }
    else if (d == W || d == E) {
        if (index / w != 0 && index / w != (w - 1)) {
            width = 1;
            height = 3;
        }
        else {
            width = 1;
            height = 2;
        }
    }
    else{
        height = 2;
        width = 2;
    }
    width *= w;
    height *= w;
    return std::make_pair(width, height);
}

//For processing pathfinding across adjacent tiles;
//index is the index of the starting TileData tile 
//g is the grid that we are pathfinding in
//d is the direction we want to move in
//s is the size of the agent
//ISSUE: THIS FUNCTION WORKS UNDER THE ASSUMPTION THAT d IS THE DIRECTION WE ARE MOVING IN, 
//BUT OTHER FUNCTIONS, LIKE getStartPos, and isPathable, WORK UNDER THE ASSUMPTION THAT d 
//IS THE DIRECTION WE ARE COMING FROM, i.e. THE OPPOSITE. NEED TO FIX THIS DISCREPANCY OTHERWISE 
//THE WHOLE CODE IS FUCKED. I THINK WE SHOULD STAY WITH d BEING THE DIRECTION WE WANT TO MOVE IN
bool isPathableWithAdjacent(int index, std::shared_ptr<GridData> g, Direction8 d, int s) {
    if (numExits(g->tiles[index]) < 2) return false;

    //Determining the correct width based on orientation
    
    // int width = getWidth(index, g->subWidth, d);
    // int height = getHeight(index, g->subWidth, d);
    std::pair<int, int> dimensions = getDimensions(index, g->subWidth, d);

    //Possible fix?
    std::vector<int> combinedCells = getCombinedSubcellGrid(index, g, d); //Get the combined subcell grid
    std::pair<int, int> startPos = getStartPos(combinedCells, dimensions.first, dimensions.second, s, getOppositeDirection(d)); //Get the start position
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

    //I think the issue is that valid positions are not considered adjacent to the edge. This is quite annoying
    std::vector<bool> prepArray = preprocessValidPositions(combinedCells, dimensions.first, s);//Get the valid positions in the array

    return pathExists(startPos.first, startPos.second, s, dimensions.first, prepArray, getOppositeDirection(d)); //Check that a valid path exists through the tile
}

//A bunch of helper bools for the trimCells function
bool isAtTopEdge(std::pair<int, int> vec) {
    return vec.second == -1;
}

bool isAtLeftEdge(std::pair<int, int> vec) {
    return vec.first == -1;
}

bool isAtRightEdge(std::pair<int, int> vec) {
    return vec.first == 1;
}

bool isAtBottomEdge(std::pair<int, int> vec) {
    return vec.second == 1;
}

//Returns a vector of Vector2s that represent all the coordinates of the 
//"valid" neighbour cells of the current index for pathfinding if moving in a certain direction
//It removes all the non-valid neighbours (those that would end up outside the grid)
//from the vector it returns
std::vector<std::pair<int, int>> trimCells(int index, int gridWidth, int gridHeight, Direction8 d) {
    //Break check on bad indeces
    if(index < 0 || index >= (gridWidth * gridHeight)) {
        return std::vector<std::pair<int, int>>();
    }

    //The basic vector contains all of the coordinates of the neighbour cells, as well as the current cell
    std::vector<std::pair<int, int>> retVec = {std::make_pair(-1, -1), std::make_pair(0, -1), std::make_pair(1, -1), 
                                               std::make_pair(-1,  0), std::make_pair(0, 0),  std::make_pair(1, 0), 
                                               std::make_pair(-1, 1),  std::make_pair(0, 1),  std::make_pair(1, 1)};
    
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

std::vector<int>* getNeighbourCells(int index, std::shared_ptr<GridData>g, std::pair<int, int> vec) {
    //Check to make sure we are only checking direct neighbours and we don't have weird neighbour vectors
    //if((abs(vec.first) != 0 || abs(vec.first) != 1 && abs(vec.second) != 0 || abs(vec.second) != 1 )) return nullptr;
    int nIndex = index+ (g->gridWidth*vec.second) + vec.first;
    //if(nIndex < 0 || nIndex > g->gridHeight*g->gridWidth) return nullptr; //Check that we are not out of bounds
    return &g->tiles[nIndex].subcells;
}

std::vector<int> createSurroundGrid(int index, std::shared_ptr<GridData> g, Direction8 d) {
    //Get the coordinates of all of the neighbours of the cell that are inside the grid
    std::vector<std::pair<int, int>> neighbours = trimCells(index, g->gridWidth, g->gridHeight, d);

    //Create the new vector:
    std::vector<std::vector<int>*> subcellArr(neighbours.size());
    std::transform(neighbours.begin(), neighbours.end(), subcellArr.begin(),
                    [index, g](std::pair<int, int> v) {return getNeighbourCells(index, g, v);});

    //Combine the neighbour and current cells together
    return combineTiles(subcellArr, g->subWidth, 3, 3);
}

bool isPathBetween(Direction8 from, Direction8 to, std::shared_ptr<GridData> g, int indexAt, int indexTo, int s) {
    //Make the grid subset that we need to path on to get to the desired direction
    std::vector<int> combinedCells = createSurroundGrid(indexAt, g, to);

    //The width of the combined grid
    //This is wrong I think also need to add height. Just use dummy variables for now, need to test other functions first
    // int width = (indexAt % g->gridWidth == 0 || indexAt % g->gridWidth == g->gridWidth-1) ? 2 : 3;
    // width *= g->subWidth;
    int width = 0;
    int height = 0;

    //Find startPosition and endPosition depending on from and to
    std::pair<int, int> startPos = getStartPos(g->tiles[indexAt].subcells, width, height, s, from); //This and the one below are also wrong I think
    if (startPos == std::make_pair(-1, -1)) return false; //Check that it is valid
    std::pair<int, int> endPos = getStartPos(g->tiles[indexTo].subcells, width, height, s, to);
    if (endPos == std::make_pair(-1, -1)) return false; //Check that it is valid

    //Need to adjust start position to work in the overall combined grid
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
