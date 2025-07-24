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

//Node constructor
Node::Node(int xPos, int yPos) : x(xPos), y(yPos), f(0), g(0), h(0), partial(false) {}

bool Node::operator>(const Node& other) const {
	return f > other.f;
}

bool Node::operator==(const Node& other) const {
	return x == other.x && y == other.y;
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
bool isValidPos(std::vector<int>& subcellArr, int w, int x, int y, int s) {
    //The x and y values represent the coords of the top-left corner of the agent. 
    //If this, plus the agent's size in subcells is outside the dimensions of the subcells grid,
    //return false
    if (x < 0 || y < 0 || x + s > w || y + s > (subcellArr.size()/ w)) //All tiles are square
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

//startX, startY are the starting x and y positions.
//endX, endY are the ending x and y positions.s
//s is the size of the agent
//w is the width of the subcell array we are working on
//pArr is the array of preprocessed valid positions
bool pathExistsTo(int startX, int startY, int endX, int endY, int s, int w, std::vector<bool>& pArr) {
    //If the start is invalid, return false
    //if (!pArr[(startY*w)+startX] ||!pArr[(endY*w)+endX]) return false;

    //Initialise the variables
    bool found = false;
    bool* visitedArr = (bool*)malloc(pArr.size() * sizeof(bool)); //Marks which nodes we have visited
    if (!visitedArr) return false;
    memset(visitedArr, false, pArr.size() * sizeof(bool)); //All nodes are initially unvisited
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
            if (newX >= 0 && newX < w && newY >= 0 && newY < (pArr.size()/w) && !visitedArr[(newY * w) + newX]) {
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

std::vector<int> combineTiles(std::vector<std::vector<int>*>& tArray, int tileW, int newGridW, int newGridH) {
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
std::vector<std::pair<int, int>> trimCells(int index, int gridWidth, int gridHeight, std::pair<int, int> *startPos) {
    //Break check on bad indeces
    if(index < 0 || index >= (gridWidth * gridHeight)) {
        return std::vector<std::pair<int, int>>();
    }

    //The basic vector contains all of the coordinates of the neighbour cells, as well as the current cell
    std::vector<std::pair<int, int>> retVec = {std::make_pair(-1, -1), std::make_pair(0, -1), std::make_pair(1, -1), 
                                               std::make_pair(-1,  0), std::make_pair(0, 0),  std::make_pair(1, 0), 
                                               std::make_pair(-1, 1),  std::make_pair(0, 1),  std::make_pair(1, 1)};
    
    
    //Trimming the cells if we are at an edge
    if(index % gridWidth == 0) { //West edge
        retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtLeftEdge), retVec.end());
        startPos->first = 0;//Need to adjust the coordinates of the starting cell  
    } 
    if(index % gridWidth == gridWidth - 1){ //East edge
        retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtRightEdge), retVec.end());
    }
    if(index / gridWidth == 0) {//North edge
        retVec.erase(std::remove_if(retVec.begin(), retVec.end(), isAtTopEdge), retVec.end());
        startPos->second = 0;//Need to adjust the coordinates of the starting cell
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

std::vector<int> createSurroundGrid(int index, std::shared_ptr<GridData> g, std::pair<int, int> *startPos, std::pair<int, int> *dimensions) {
    //Get the coordinates of all of the neighbours of the cell that are inside the grid
    std::vector<std::pair<int, int>> neighbours = trimCells(index, g->gridWidth, g->gridHeight, startPos);
    //std::pair<int, int> dimensions = std::make_pair(0,0);

    if(neighbours.size() == 4) {
        dimensions->first = 2;
        dimensions->second =2;
    }
    else if (neighbours.size() == 6) {
        if(index % g->gridWidth == 0 || index % g->gridWidth == g->gridWidth -1) {
            dimensions->first = 2;
            dimensions->second = 3;
        }
        else {
            dimensions->first = 3;
            dimensions->second = 2;
        }
    }
    else {
        dimensions->first = 3;
        dimensions->second = 3;
    }

    //Create the new vector:
    std::vector<std::vector<int>*> subcellArr(neighbours.size());
    std::transform(neighbours.begin(), neighbours.end(), subcellArr.begin(),
                    [index, g](std::pair<int, int> v) {return getNeighbourCells(index, g, v);});

    //Combine the neighbour and current cells together
    return combineTiles(subcellArr, g->subWidth, dimensions->first, dimensions->second);
}

std::pair<int, int> getRestrictedStartPos(std::vector<int>& subcellArr, int w, int gW, int offsetX, int offsetY, int s, Direction8 d) {
    switch (d) {
    case NW:
        if (isValidPos(subcellArr, gW, offsetX, offsetY, s)) return std::make_pair(offsetX, offsetY);
        return std::make_pair(-1, -1);
    case N:
        for (int x = 0; x < w; x++) {
            if (isValidPos(subcellArr, gW, x+offsetX, offsetY, s)) return std::make_pair(x+offsetX, offsetY);
        }
        return std::make_pair(-1, -1);
    case NE:
        if (isValidPos(subcellArr, gW, w - s +offsetX, offsetY, s)) return std::make_pair(w - s+offsetX, offsetY);
        return std::make_pair(-1, -1);
    case E:
        for (int y = 0; y < w; y++) {
            if (isValidPos(subcellArr, gW, w - s+offsetX, y+offsetY, s)) return std::make_pair(w - s+offsetX, y+offsetY);
        }
        return std::make_pair(-1, -1);
    case SE:
        if (isValidPos(subcellArr, gW, w - s+offsetX, w - s+offsetY, s)) return std::make_pair(w - s+offsetX, w - s+offsetY);
        return std::make_pair(-1, -1);
    case S:
        for (int x = 0; x < w; x++) {
            if (isValidPos(subcellArr, gW, x+offsetX, w - s+offsetY, s)) return std::make_pair(x+offsetX, w - s+offsetY);
        }
        return std::make_pair(-1, -1);
    case SW:
        if (isValidPos(subcellArr, gW, offsetX, w - s+offsetY, s)) return std::make_pair(offsetX, w - s+offsetY);
        return std::make_pair(-1, -1);
    case W:
        for (int y = 0; y < w; y++) {
            if (isValidPos(subcellArr, gW, offsetX, y+offsetY, s)) return std::make_pair(offsetX, y+offsetY);
        }
        return std::make_pair(-1, -1);
    }
}

std::pair<int, int> getRestrictedEndPos(std::vector<int>& subcellArr, int w, int gW, int offsetX, int offsetY, int s, Direction8 d) {
    switch (d) {
    case NW:
        if(s == 1 && !isValidPos(subcellArr, gW, offsetX, offsetY - 1, s) && !isValidPos(subcellArr, gW, offsetX-1, offsetY, s))
            return std::make_pair(-1, -1);
        if (isValidPos(subcellArr, gW, offsetX-1, offsetY-1, s)) return std::make_pair(offsetX-1, offsetY-1);
        return std::make_pair(-1, -1);
    case N:
        for (int x = 0; x < w; x++) {
            if (s == 1 && !isValidPos(subcellArr, gW, x + offsetX, offsetY, s)) continue;
            if (isValidPos(subcellArr, gW, x+offsetX, offsetY-1, s)) return std::make_pair(x+offsetX, offsetY-1);
        }
        return std::make_pair(-1, -1);
    case NE:
        if (s == 1 && !isValidPos(subcellArr, gW, w - s + offsetX, offsetY - 1, s) && !isValidPos(subcellArr, gW, w - s + offsetX + 1, offsetY, s))
            return std::make_pair(-1, -1);
        if (isValidPos(subcellArr, gW, w - s +offsetX+1, offsetY-1, s)) return std::make_pair(w - s+offsetX+1, offsetY-1);
        return std::make_pair(-1, -1);
    case E:
        for (int y = 0; y < w; y++) {
            if (s == 1 && !isValidPos(subcellArr, gW, w - s + offsetX, y + offsetY, s)) continue;
            if (isValidPos(subcellArr, gW, w - s+offsetX+1, y+offsetY, s)) return std::make_pair(w - s+offsetX+1, y+offsetY);
        }
        return std::make_pair(-1, -1);
    case SE:
        if (s == 1 && !isValidPos(subcellArr, gW, w - s + offsetX, w - s + offsetY + 1, s) && !isValidPos(subcellArr, gW, w - s + offsetX + 1, w - s + offsetY, s))
            return std::make_pair(-1, -1);
        if (isValidPos(subcellArr, gW, w - s+offsetX+1, w - s+offsetY+1, s)) return std::make_pair(w - s+offsetX+1, w - s+offsetY+1);
        return std::make_pair(-1, -1);
    case S:
        for (int x = 0; x < w; x++) {
            if (s == 1 && !isValidPos(subcellArr, gW, x + offsetX, w - s + offsetY, s)) continue;
            if (isValidPos(subcellArr, gW, x+offsetX, w - s+offsetY+1, s)) return std::make_pair(x+offsetX, w - s+offsetY+1);
        }
        return std::make_pair(-1, -1);
    case SW:
        if (s == 1 && !isValidPos(subcellArr, gW, offsetX, w - s + offsetY + 1, s) && !isValidPos(subcellArr, gW, offsetX - 1, w - s + offsetY, s))
            return std::make_pair(-1, -1);
        if (isValidPos(subcellArr, gW, offsetX-1, w - s+offsetY+1, s)) return std::make_pair(offsetX-1, w - s+offsetY+1);
        return std::make_pair(-1, -1);
    case W:
        for (int y = 0; y < w; y++) {
            if (s == 1 && !isValidPos(subcellArr, gW, offsetX, y+ offsetY, s)) continue;
            if (isValidPos(subcellArr, gW, offsetX-1, y+offsetY, s)) return std::make_pair(offsetX-1, y+offsetY);
        }
        return std::make_pair(-1, -1);
    }
}

//Returns an array of positions where an agent of size s*s can stand in a subcell grid
std::vector<bool> preprocessRestrictedValidPositions(std::vector<int> subcellArr, int w, int gW, int s, int offsetX, int offsetY) {
    std::vector<bool> retArr(subcellArr.size(), false);//The array being returned. Initially assume all positions are impassible
    for (int i = -1; i <= w; i++) {//x position
        for (int j = -1; j <= w; j++) {//y position
            //This should avoid the issue of agents of size 1 being able to "go around" impassible subcells
            //by going through passible subcells on neighbouring tiles to reach the desired target, which 
            //would seriously mess up the A*
            if (j + offsetY < 0 || i + offsetX < 0 || i + offsetX >= gW || j + offsetY >= subcellArr.size()/gW ) continue;
            if(s == 1) 
                if(i == -1 && subcellArr[((j+offsetY)*gW) + offsetX] == 1||
                   i == w && subcellArr[((j+offsetY)*gW) + w-1 + offsetX] == 1||
                   j == -1 && subcellArr[(offsetY*gW) + i+offsetX] == 1 ||
                   j == w && subcellArr[((j-1+offsetY)*gW) + i + offsetX] == 1) 
                    continue;
            //If this position is passable by the agent, can set it to true
            if (isValidPos(subcellArr, gW, i+offsetX, j+offsetY, s))
                retArr[((j+offsetY) * gW) + i+offsetX] = true;
        }
    }
    return retArr;
}

//This function finds whether there is a path using the current cell to the neighbour at a specific direciton
//from represents where we are entering the current cell from
//to represents where we want to exit the cell to (to reach the desired neighbour)
//g is the GridData pointer
//indexAt is the index of the current TileData in g
//s is the size of the agent passing through
bool isPathBetween(Direction8 from, Direction8 to, std::shared_ptr<GridData> g, int indexAt, int s) {
    std::pair<int, int> dimensions = std::make_pair(0,0);
    std::pair<int, int> startTile = std::make_pair(1, 1); //Represents the coordinates of the starting tile in the 3x3 grid where (0,0) is the top left

    //Make the grid subset that we need to path on to get to the desired direction
    std::vector<int> combinedCells = createSurroundGrid(indexAt, g, &startTile, &dimensions);

    int offsetX = startTile.first*g->subWidth;
    int offsetY = startTile.second*g->subWidth;    

    //Find startPosition and endPosition depending on from and to
    std::pair<int, int> startPos = getRestrictedStartPos(combinedCells, g->subWidth, g->subWidth * dimensions.first, offsetX, offsetY, s, from);
    if (startPos == std::make_pair(-1, -1)) return false; //Check that it is valid
    std::pair<int, int> endPos = getRestrictedEndPos(combinedCells, g->subWidth, g->subWidth * dimensions.first, offsetX, offsetY, s, to);
    if (endPos == std::make_pair(-1, -1)) return false; //Check that it is valid

    //Find if there is a path between them 
    std::vector<bool> prepArray = preprocessRestrictedValidPositions(combinedCells, g->subWidth, g->subWidth*dimensions.first, s, offsetX, offsetY);//Get the valid positions in the array
    return pathExistsTo(startPos.first, startPos.second, endPos.first, endPos.second, s, dimensions.first*g->subWidth, prepArray); //Check that a valid path exists through the tile
}

//Gets the relevant node from the world position
Node nodeFromWorldPos(Vector2 pos, int w, int h) {
	return Node(static_cast<int>(floor(pos.x / w)), static_cast<int>(floor(pos.y / h)));
}   

//Converts a node's grid position to its world position
Vector2 nodeToWorldPos(Node n, int w) {
    return Vector2{
        n.x * w + w / 2.0f,
        n.y * w + w / 2.0f
    };
}

//Gets the distance between two nodes as the crow flies
int getDistance(Node a, Node b) {
    int dstX = (int)abs(a.x - b.x);
    int dstY = (int)abs(a.y - b.y);
    if (dstX > dstY)
        return 14 * dstY + 10 * (dstX - dstY);
    return 14 * dstX + 10 * (dstY - dstX);
}

//Need to fix diagonals when going from empty to empty node in this one -> have to check the partial way
std::vector<Node> FindPath(Vector2 start, Vector2 goal, std::shared_ptr<GridData> grid, int size) {
    //Represents the (x,y) coordinates of all possible neighbours
    const int directionX[] = { -1, 0, 1, 0, 1, 1, -1, -1 };
    const int directionY[] = { 0, 1, 0, -1, 1, -1, 1, -1 };

    std::unordered_map<Vector2, Direction8, Vector2Hasher> directionMap = {{Vector2(-1, -1), NW}, {Vector2(0, -1), N},
                                                            {Vector2(1, -1), NE},  {Vector2(-1, 0), W},
                                                            {Vector2(1, 0), E},   {Vector2(-1, 1), SW}, 
                                                            {Vector2(0, 1), S},   {Vector2(1, 1), SE}};

    const int straightCost = 10; //Cost of moving straight -> 1* 10
    const int diagonalCost = 14; //Cost of moving diagonally ~sqrt(2) *10

    int rows = grid->gridHeight;
    int cols = grid->gridWidth;

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openList; //Nodes to visit
    std::unordered_set<Node, NodeHasher> closedList; //Nodes visited
    std::unordered_map<Node, Node, NodeHasher> cameFrom; //Holds the parents of each node -> the one visited before 
    std::vector<int> gScore(rows * cols, INT_MAX);//Holds the gScore of every node 

    // Initialize start node
    Node startNode = nodeFromWorldPos(start, grid->tileWidth, grid->tileHeight);
    Node goalNode = nodeFromWorldPos(goal, grid->tileWidth, grid->tileHeight);
    startNode.g = 0;
    startNode.h = getDistance(startNode, goalNode);
    startNode.f = startNode.g + startNode.h;

    gScore[toIndex(grid, Vector2(startNode.x, startNode.y))] = 0;
    openList.push(startNode);

    //While there are nodes to visit
    while (!openList.empty()) {
        //Since openList is a pq, this will get the node with the lowest f score
        Node current = openList.top();
        openList.pop();

        //If we have reached the end
        if (current == goalNode) {
            //Get all of the nodes in the path from the start to the end
            std::vector<Node> path;
            Node trace = current;
            while (!(trace == startNode)) {
                path.push_back(trace);
                trace = cameFrom[trace];
            }
            path.push_back(startNode);
            //Reverse it so it is in the correct order
            reverse(path.begin(), path.end());
            return path;
        }
        //Otherwise mark the current node as visited
        closedList.insert(current);
        
        std::vector<Node> goodNeighbours; //For holding neighbour nodes to be processed and added to the openList

        if (grid->tiles[toIndex(grid, Vector2(current.x, current.y))].status == 0) {
            //Visit all of its neighbours and insert them into the openList
            for (int i = 0; i < 8; ++i) {
                //The neighbour's coordinates
                int newX = current.x + directionX[i];
                int newY = current.y + directionY[i];

                //Making sure we don't go out of grid bounds and crash the program
                if (newX >= 0 && newX < cols && newY >= 0 && newY < rows) {
                    int index = toIndex(grid, Vector2(newX, newY));
                    if (grid->tiles[index].status == 0) { //If neighbour is walkable
                        //If we are walking to a neighbour that is walkable, but diagonally adjacent, 
                        //need to manually check if there is a path between these tiles, otherwise 
                        //the agent will get stuck ramming itself into a corner.
                        if (directionX[i] != 0 && directionY[i] != 0) {
                            auto direction = directionMap.at(Vector2(directionX[i], directionY[i]));
                            auto directionFrom = (current == startNode) ? S : directionMap.at(Vector2(cameFrom[current].x - current.x, cameFrom[current].y - current.y));
                            if (isPathBetween(directionFrom, direction, grid, toIndex(grid, Vector2(current.x, current.y)), size)) {
                                goodNeighbours.push_back(Node(newX, newY));
                            }
                        }
                        else {
                            goodNeighbours.push_back(Node(newX, newY));
                        }
                    }

                    if (grid->tiles[index].status == 2) { //If neighbour is partial
                        auto direction = directionMap.at(Vector2(directionX[i], directionY[i]));
                        auto directionFrom = (current == startNode) ? S : directionMap.at(Vector2(cameFrom[current].x - current.x, cameFrom[current].y - current.y));
                        if (isPathBetween(directionFrom, direction, grid, toIndex(grid, Vector2(current.x, current.y)), size)) {
                            goodNeighbours.push_back(Node(newX, newY));
                        }
                    }
                }
            }
        }
        else {
            const TileData& t = grid->tiles[toIndex(grid, Vector2(current.x, current.y))];
            for(int i = 0; i < 8; i++) {
                //The neighbour's coordinates
                int newX = current.x + directionX[i];
                int newY = current.y + directionY[i];

                //Making sure we don't go out of grid bounds and crash the program
                if (newX >= 0 && newX < cols && newY >= 0 && newY < rows) {
                    auto direction = directionMap.at(Vector2(directionX[i], directionY[i]));
                    auto directionFrom = (current == startNode) ? S : directionMap.at(Vector2(cameFrom[current].x - current.x, cameFrom[current].y - current.y));
                    //Need to figure out how to get direction that we are coming from.
                    if(isPathBetween(directionFrom, direction, grid, toIndex(grid, Vector2(current.x, current.y)), size)) {
                        goodNeighbours.push_back(Node(newX, newY));
                    }
                }
            }
        }

        //Deferring adding nodes to the openList to the end to avoid copying code in this function
        for(auto neighbor : goodNeighbours) {
            //If it is not already visited
            if (closedList.find(neighbor) == closedList.end()) {
                int moveCost = ((current.x - neighbor.x) != 0 && (current.y -neighbor.y) != 0) ? diagonalCost : straightCost;
                int index = toIndex(grid, Vector2(neighbor.x, neighbor.y));
                int tentativeG = gScore[toIndex(grid, Vector2(current.x, current.y))] + moveCost;

                if (tentativeG < gScore[index]) {
                    gScore[index] = tentativeG;
                    neighbor.g = tentativeG;
                    neighbor.h = getDistance(neighbor, goalNode);
                    neighbor.f = neighbor.g + neighbor.h;
                    cameFrom[neighbor] = current;
                    openList.push(neighbor);
                }
            }
        }
    }

    return std::vector<Node>(); // No path found
}

//Original pure A* implementation:
std::vector<Node> FindPathAStar(Vector2 start, Vector2 goal, std::shared_ptr<GridData> grid) {
    //Represents the (x,y) coordinates of all possible neighbours
    const int directionX[] = { -1, 0, 1, 0, 1, 1, -1, -1 };
    const int directionY[] = { 0, 1, 0, -1, 1, -1, 1, -1 };
    
    const int straightCost = 10; //Cost of moving straight -> 1* 10
    const int diagonalCost = 14; //Cost of moving diagonally ~sqrt(2) *10

    int rows = grid->gridHeight;
    int cols = grid->gridWidth;

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openList; //Nodes to visit
    std::unordered_set<Node, NodeHasher> closedList; //Nodes visited
    std::unordered_map<Node, Node, NodeHasher> cameFrom; //Holds the parents of each node -> the one visited before 
    std::vector<int> gScore(rows * cols, INT_MAX);//Holds the gScore of every node 

    // Initialize start node
    Node startNode = nodeFromWorldPos(start, grid->tileWidth, grid->tileHeight);
    Node goalNode = nodeFromWorldPos(goal, grid->tileWidth, grid->tileHeight);
    startNode.g = 0;
    startNode.h = getDistance(startNode, goalNode);
    startNode.f = startNode.g + startNode.h;

    gScore[toIndex(grid, Vector2(startNode.x, startNode.y))] = 0;
    openList.push(startNode);

    //While there are nodes to visit
    while (!openList.empty()) {
        //Since openList is a pq, this will get the node with the lowest f score
        Node current = openList.top();
        openList.pop();

        //If we have reached the end
        if (current == goalNode) {
            //Get all of the nodes in the path from the start to the end
            std::vector<Node> path;
            Node trace = current;
            while (!(trace == startNode)) {
                path.push_back(trace);
                trace = cameFrom[trace];
            }
            path.push_back(startNode);
            //Reverse it so it is in the correct order
            reverse(path.begin(), path.end());
            return path;
        }

        //Otherwise mark the current node as visited
        closedList.insert(current);

        //Visit all of its neighbours and insert them into the openList
        for (int i = 0; i < 8; ++i) {
            //The neighbour's coordinates
            int newX = current.x + directionX[i];
            int newY = current.y + directionY[i];

            //Making sure we don't go out of grid bounds and crash the program
            if (newX >= 0 && newX < cols && newY >= 0 && newY < rows) {
                if (grid->tiles[toIndex(grid, Vector2(newX, newY))].status == 0) { //If it walkable
                    Node neighbor(newX, newY);
                    //If it is not already visited
                    if (closedList.find(neighbor) == closedList.end()) {
                        int moveCost = (directionX[i] != 0 && directionY[i] != 0) ? diagonalCost : straightCost;

                        int tentativeG = gScore[toIndex(grid, Vector2(current.x, current.y))] + moveCost;

                        if (tentativeG < gScore[toIndex(grid, Vector2(newX, newY))]) {
                            gScore[toIndex(grid, Vector2(newX, newY))] = tentativeG;
                            neighbor.g = tentativeG;
                            neighbor.h = getDistance(neighbor, goalNode);
                            neighbor.f = neighbor.g + neighbor.h;
                            cameFrom[neighbor] = current;
                            openList.push(neighbor);
                        }
                    }
                }
            }
        }
    }

    return std::vector<Node>(); // No path found
}