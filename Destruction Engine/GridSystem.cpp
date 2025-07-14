#include "GridSystem.h"
#include <algorithm>
#include <unordered_set>

const std::unordered_map<Vector2, Direction8>& getDirectionMap() {
    std::unordered_map<Vector2, Direction8> directionMap = {{Vector2(-1, -1), NW}, {Vector2(0, -1), N},
                                                            {Vector2(1, -1), NE},  {Vector2(-1, 0), E},
                                                            {Vector2(1, 0), SE},   {Vector2(-1, 1), S}, 
                                                            {Vector2(0, 1), SW},   {Vector2(1, 1), W}};
    return directionMap;
}

//Initialises the signature for the grid system and creates the tiles for the grid
void GridSystem::init() {
    //Load and create the tiles
    //std::vector<int> tileGrid = loadTiles();
	createTiles();
    //grid = convertTilesToGrid();
}

//Checks if the grid has changed in a meaningful way (walkable status of tiles), and if it has, publishes an
//event so the pathfinding system can be updated too
void GridSystem::update(float dt) {
	bool gridChanged = false;
	for (Entity e : registeredEntities) {
		if (tileStatusChanged(e)) {
			gridChanged = true;
		}
	}

	if (gridChanged) {
		//gCoordinator.getEventBus()->publish(new GridChangedEvent(convertTilesToGrid()));
	}
}

//Creates the tiles for the grid
void GridSystem::createTiles() {
    for (int i = 0; i < grid->gridHeight; i++) {
        for (int j = 0; j < grid->gridWidth; j++) {
            Entity e = gCoordinator.createEntity();
            gCoordinator.addComponent(e, Transform(Vector2((j * TILE_WIDTH) + (TILE_WIDTH / 2), ( i* TILE_HEIGHT) + (TILE_HEIGHT / 2)), 0));
            gCoordinator.addComponent(e, Walkable(grid->tiles[toIndex(grid, Vector2(j, i))].status));
        }
	}
}

void GridSystem::setGrid(std::shared_ptr<GridData> g) {
    grid = g;
}

//This only needs to be filled once the actual destruction pathfinding system has been implemented
bool GridSystem::tileStatusChanged(Entity e) {
	return false;
}

//Converts the tiles into their marching squares representation as a 2D vector so we can do pathfinding on them
std::vector<std::vector<int>> GridSystem::convertTilesToGrid() {
    //std::vector<std::vector<int>> tempGrid( static_cast<size_t>(gridWidthInTiles * 2), std::vector<int>(static_cast<size_t>(gridHeightInTiles * 2), -1) );
    std::vector<std::vector<int>> tempGrid(static_cast<size_t>(grid->gridWidth * 2), std::vector<int>(static_cast<size_t>(grid->gridHeight * 2), -1));
    for (Entity e : registeredEntities) {
        Transform t = gCoordinator.getComponent<Transform>(e);
        Walkable w = gCoordinator.getComponent<Walkable>(e);

        //Origin of each set of four is in the top left
        int topX = static_cast<int>(floor(t.position.x / TILE_WIDTH))*2;
        int topY = static_cast<int>(floor(t.position.y / TILE_HEIGHT))*2;

        //Manual conversion of the a* tiles to marching squares representation in the grid for proper pathfinding
        switch (w.walkStatus) {
            case 0:
                tempGrid[topX][topY] = 0;
                tempGrid[topX+1][topY] = 0;
                tempGrid[topX][topY+1] = 0;
                tempGrid[topX+1][topY+1] = 0;
            break;
            case 1:
                tempGrid[topX][topY] = 0;
                tempGrid[topX + 1][topY] = 0;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 0;
            break;
            case 2:
                tempGrid[topX][topY] = 0;
                tempGrid[topX + 1][topY] = 0;
                tempGrid[topX][topY + 1] = 0;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
            case 3:
                tempGrid[topX][topY] = 0;
                tempGrid[topX + 1][topY] = 0;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
            case 4:
                tempGrid[topX][topY] = 0;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 0;
                tempGrid[topX + 1][topY + 1] = 0;
            break;
            case 5:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 0;
                tempGrid[topX][topY + 1] = 0;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
            case 6:
                tempGrid[topX][topY] = 0;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 0;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
            case 7:
                tempGrid[topX][topY] = 0;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
            case 8:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 0;
                tempGrid[topX][topY + 1] = 0;
                tempGrid[topX + 1][topY + 1] = 0;
            break;
            case 9:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 0;
                tempGrid[topX + 1][topY + 1] = 0;
            break;
            case 10:
                tempGrid[topX][topY] = 0;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 0;
            break;
            case 11:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 0;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
            case 12:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 0;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 0;
            break;
            case 13:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 0;
            break;
            case 14:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 0;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
            case 15:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
        }
    }

    /*for (int j = 0; j < tempGrid.size(); j++) {
        for (int i = 0; i < tempGrid[j].size(); i++) {
            printf("%i ", tempGrid[j][i]);
        }
        printf("\n");
    }*/

    return tempGrid;
}

//Node constructor
Node::Node(int xPos, int yPos) : x(xPos), y(yPos), f(0), g(0), h(0), partial(false) {}

bool Node::operator>(const Node& other) const {
	return f > other.f;
}

bool Node::operator==(const Node& other) const {
	return x == other.x && y == other.y;
}

//Pathfinding system initialisation. Incomplete
void PathFindingSystem::init() {

}

void PathFindingSystem::update(float dt) {
    //Need to find a better way of doing this. This is just a quick version that works for now. Use a queue maybe?
    //And then have a function that adds PathFinding requests Entities to the queue or something
    for (Entity e : registeredEntities) {
        Pathfinding &p = gCoordinator.getComponent<Pathfinding>(e);

        //If the component does not have a well defined start and end pos, continue
        if (p.startPos == Vector2() || p.endPos == Vector2()) {
            continue;
        }
        else {
            //This code also appends end and startPos to the list:
            //Probably need to use rdp on it to smooth out the path
            /*
                vecPath.reserve(path.size() + 2);  // Optional optimization
                
                // Add start position
                vecPath.push_back(p.startPos);

                // Convert internal path to world coordinates
                std::transform(path.begin(), path.end(), std::back_inserter(vecPath),
                    [this](const Node& n) { return nodeToWorldPos(n); });

                // Add end position
                vecPath.push_back(p.endPos);
            */

            //Get the path in node form
            auto path = FindPath(p.startPos, p.endPos);
            std::vector<Vector2> vecPath(path.size());

            //Convert it to coordinates
            std::transform(path.begin(), path.end(), vecPath.begin(),
                [this](Node n) {return nodeToWorldPos(n); });

            for (Vector2 v : vecPath) {
                printf("(%f, %f)", v.x, v.y);
            }

            //Set the path value in the component
            p.path = vecPath;
            //Reset the start and endpos to default values to avoid repeat computations
            p.startPos = Vector2();
            p.endPos = Vector2();
        }
    }
}

void PathFindingSystem::setGrid(std::shared_ptr<GridData> g) {
    grid = g;
}

//Original pure A* implementation:
std::vector<Node> PathFindingSystem::FindPath(Vector2 start, Vector2 goal) {
    //Represents the (x,y) coordinates of all possible neighbours
    const int directionX[] = { -1, 0, 1, 0, 1, 1, -1, -1 };
    const int directionY[] = { 0, 1, 0, -1, 1, -1, 1, -1 };
    
    const int straightCost = 10; //Cost of moving straight -> 1* 10
    const int diagonalCost = 14; //Cost of moving diagonally ~sqrt(2) *10

    int rows = grid->gridHeight;
    int cols = grid->gridWidth;

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openList; //Nodes to visit
    std::unordered_set<Node> closedList; //Nodes visited
    std::unordered_map<Node, Node, std::hash<Node>> cameFrom; //Holds the parents of each node -> the one visited before 
    std::vector<int> gScore(rows * cols, INT_MAX);//Holds the gScore of every node 

    // Initialize start node
    Node startNode = nodeFromWorldPos(start);
    Node goalNode = nodeFromWorldPos(goal);
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

//Modified pathfinding for destructability:
std::vector<Node> PathFindingSystem::FindPath2(Vector2 start, Vector2 goal) {
    //Represents the (x,y) coordinates of all possible neighbours
    const int directionX[] = { -1, 0, 1, 0, 1, 1, -1, -1 };
    const int directionY[] = { 0, 1, 0, -1, 1, -1, 1, -1 };

    const int straightCost = 10; //Cost of moving straight -> 1* 10
    const int diagonalCost = 14; //Cost of moving diagonally ~sqrt(2) *10

    int rows = grid->gridHeight;
    int cols = grid->gridWidth;

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openList; //Nodes to visit
    std::unordered_set<Node> closedList; //Nodes visited
    std::unordered_map<Node, Node, std::hash<Node>> cameFrom; //Holds the parents of each node -> the one visited before 
    std::vector<int> gScore(rows * cols, INT_MAX);//Holds the gScore of every node 

    // Initialize start node
    Node startNode = nodeFromWorldPos(start);
    Node goalNode = nodeFromWorldPos(goal);
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
                int index = toIndex(grid, Vector2(newX, newY));
                if (grid->tiles[index].status == 0) { //If neighbour is walkable
                    Node neighbor(newX, newY);
                    //If it is not already visited
                    if (closedList.find(neighbor) == closedList.end()) {
                        int moveCost = (directionX[i] != 0 && directionY[i] != 0) ? diagonalCost : straightCost;

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

                if (grid->tiles[index].status == 2) { //If neighbour is partial
                    //Need to determine direction we are moving in and if it is open on the partial tile
                    //Then we need to determine if it has any other exits other than on this direction
                    //Then we need to determine if it is pathable by this agent
                    auto direction = getDirectionMap().at(Vector2(newX, newY));
                    if (isPathable(grid->tiles[index], direction/*directionMap[Vector2(newX, newY)]*/, 2)) {
                        Node neighbor(newX, newY);
                        //If it is not already visited
                        if (closedList.find(neighbor) == closedList.end()) {
                            int moveCost = (directionX[i] != 0 && directionY[i] != 0) ? diagonalCost : straightCost;

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
            }
        }
    }

    return std::vector<Node>(); // No path found
}

//Gets the relevant node from the world position
Node PathFindingSystem::nodeFromWorldPos(Vector2 pos) {
	return Node(static_cast<int>(floor(pos.x / TILE_WIDTH)), static_cast<int>(floor(pos.y / TILE_HEIGHT)));
}   

//Converts a node's grid position to its world position
Vector2 PathFindingSystem::nodeToWorldPos(Node n) {
    return Vector2{
        n.x * TILE_WIDTH + TILE_WIDTH / 2.0f,
        n.y * TILE_HEIGHT + TILE_HEIGHT / 2.0f
    };
}

//Gets the distance between two nodes as the crow flies
int PathFindingSystem::getDistance(Node a, Node b) {
    int dstX = (int)abs(a.x - b.x);
    int dstY = (int)abs(a.y - b.y);
    if (dstX > dstY)
        return 14 * dstY + 10 * (dstX - dstY);
    return 14 * dstX + 10 * (dstY - dstX);
}