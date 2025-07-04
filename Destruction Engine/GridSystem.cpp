#include "GridSystem.h"
#include <algorithm>
#include <unordered_set>

//Initialises the signature for the grid system and creates the tiles for the grid
void GridSystem::init() {
    //Load and create the tiles
    loadTiles();
	createTiles();
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
		gCoordinator.getEventBus()->publish(new GridChangedEvent(convertTilesToGrid()));
	}
}

//Creates the tiles for the grid
void GridSystem::createTiles() {
    for (int i = 0; i < gridHeightInTiles; i++) {
        for (int j = 0; j < gridWidthInTiles; j++) {
            Entity e = gCoordinator.createEntity();
            gCoordinator.addComponent(e, Transform(newVector2((j * TILE_WIDTH) + (TILE_WIDTH / 2), ( i* TILE_HEIGHT) + (TILE_HEIGHT / 2)), 0));
            gCoordinator.addComponent(e, Walkable(grid[j][i]));
        }
	}
}

void GridSystem::updatePathfinding() {
    gCoordinator.getEventBus()->publish(new GridChangedEvent(grid));
}

//Loads the tilemap from a .map file
void GridSystem::loadTiles() {
    std::ifstream map("assets/Pathfinding.map");

    if (map.fail()) {
        printf("Unable to get grid data.\n");
    }
    else {
        bool tilesLoaded = true;
        for (int i = 0; i < gridHeightInTiles; i++) {
            for (int j = 0; j < gridWidthInTiles; j++) {
                int tileType = -1;
                map >> tileType;

                //If there was a problem in reading the map 
                if (map.fail()) {
                    //Stop loading the map 
                    printf("Error loading the map: Unexpected end of file!\n");
                    tilesLoaded = false;
                    break;
                }
                
                //If the number is a valid tile number
                if ((tileType >= 0) && (tileType < TOTAL_TILE_SPRITES)) {
                    //Make a new tile
                    grid[j][i] = tileType;
                }
                else {
                    //Stop loading the map 
                    printf("Error loading map: Invalid tile type\n");
                    tilesLoaded = false;
                    break;
                }
            }
            if (!tilesLoaded) {
                break;
            }
        }
    }
}

//This only needs to be filled once the actual destruction pathfinding system has been implemented
bool GridSystem::tileStatusChanged(Entity e) {
	return false;
}

//Converts the tiles into a vector so that they can easily be stored as data
std::vector<std::vector<int>> GridSystem::convertTilesToGrid() {
    return std::vector<std::vector<int>>();
}

//Node constructor
Node::Node(int xPos, int yPos) : x(xPos), y(yPos), f(0), g(0), h(0) {}

bool Node::operator>(const Node& other) const {
	return f > other.f;
}

bool Node::operator==(const Node& other) const {
	return x == other.x && y == other.y;
}

//Pathfinding system initialisation. Incomplete
void PathFindingSystem::init() {
	gCoordinator.getEventBus()->subscribe(this, &PathFindingSystem::updateGrid);
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
            auto path = FindPath(grid, p.startPos, p.endPos);
            std::vector<Vector2> vecPath(path.size());

            //Convert it to coordinates
            std::transform(path.begin(), path.end(), vecPath.begin(),
                [this](Node n) {return nodeToWorldPos(n); });

            //Set the path value in the component
            p.path = vecPath;
            //Reset the start and endpos to default values to avoid repeat computations
            p.startPos = Vector2();
            p.endPos = Vector2();
        }
    }
}

std::vector<Node> PathFindingSystem::FindPath(const std::vector<std::vector<int>> graph, Vector2 start, Vector2 goal) {
    //Represents the (x,y) coordinates of all possible neighbours
    const int directionX[] = { -1, 0, 1, 0, 1, 1, -1, -1 };
    const int directionY[] = { 0, 1, 0, -1, 1, -1, 1, -1 };
    
    const int straightCost = 10; //Cost of moving straight -> 1* 10
    const int diagonalCost = 14; //Cost of moving diagonally ~sqrt(2) *10

    int rows = graph.size();
    int cols = graph[0].size();

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openList; //Nodes to visit
    std::unordered_set<Node> closedList; //Nodes visited
    std::unordered_map<Node, Node, std::hash<Node>> cameFrom; //Holds the parents of each node -> the one visited before them
    std::vector<std::vector<int>> gScore(rows, std::vector<int>(cols, INT_MAX)); //Holds the gScore of every node 

    // Initialize start node
    Node startNode = nodeFromWorldPos(start);
    Node goalNode = nodeFromWorldPos(goal);
    startNode.g = 0;
    startNode.h = getDistance(startNode, goalNode);
    startNode.f = startNode.g + startNode.h;

    gScore[startNode.x][startNode.y] = 0;
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
            if (newX >= 0 && newX < rows && newY >= 0 && newY < cols) {
                if (graph[newX][newY] == 0) { //If it walkable
                    Node neighbor(newX, newY);
                    //If it is not already visited
                    if (closedList.find(neighbor) == closedList.end()) {
                        int moveCost = (directionX[i] != 0 && directionY[i] != 0) ? diagonalCost : straightCost;

                        int tentativeG = gScore[current.x][current.y] + moveCost;

                        if (tentativeG < gScore[newX][newY]) {
                            gScore[newX][newY] = tentativeG;

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

//Updates the grid we have to work with
void PathFindingSystem::updateGrid(const GridChangedEvent* event) {
	grid = event->grid;
}