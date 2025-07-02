#include "GridSystem.h"
#include <algorithm>

//Initialises the signature for the grid system and creates the tiles for the grid
void GridSystem::init() {
	Signature sig;
	sig.addComponent<Transform>();
	sig.addComponent<Walkable>();
	gCoordinator.setSystemSignature<GridSystem>(sig);
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
    for (int i = 0; i < gridWidthInTiles; i++) {
        for (int j = 0; j < gridHeightInTiles; j++) {
            Entity e = gCoordinator.createEntity();
            gCoordinator.addComponent(e, Transform(newVector2((i * TILE_WIDTH) + (TILE_WIDTH / 2), (j * TILE_HEIGHT) + (TILE_HEIGHT / 2)), 0));
            gCoordinator.addComponent(e, Walkable(grid[i][j]));
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

//Not sure what components to use for PathFinding update. Maybe just attach a pathfinding component that holds
//start and end pos?
void PathFindingSystem::update(float dt) {

}

std::vector<Node> PathFindingSystem::FindPath(const std::vector<std::vector<int>> graph, Vector2 start, Vector2 goal) {
    const int directionX[] = { -1, 0, 1, 0 };
    const int directionY[] = { 0, 1, 0, -1 };

    int rows = graph.size();
    int cols = graph[0].size();

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openList;
    std::vector<std::vector<bool>> closedList(rows, std::vector<bool>(cols, false));
    std::vector<std::vector<Node>> cameFrom(rows, std::vector<Node>(cols, Node(-1, -1)));
    std::vector<std::vector<int>> gScore(rows, std::vector<int>(cols, INT_MAX));

    // Initialize start node
    Node startNode = nodeFromWorldPos(start);
    Node goalNode = nodeFromWorldPos(goal);
    startNode.g = 0;
    startNode.h = abs(start.x - goal.x) + abs(start.y - goal.y);
    startNode.f = startNode.g + startNode.h;

    gScore[startNode.x][startNode.y] = 0;
    openList.push(startNode);

    while (!openList.empty()) {
        Node current = openList.top();
        openList.pop();

        if (current == goalNode) {
            std::vector<Node> path;
            Node trace = current;
            while (!(trace == startNode)) {
                path.push_back(trace);
                trace = cameFrom[trace.x][trace.y];
            }
            path.push_back(startNode);
            reverse(path.begin(), path.end());
            return path;
        }

        if (closedList[current.x][current.y])
            continue;

        closedList[current.x][current.y] = true;

        for (int i = 0; i < 4; ++i) {
            int newX = current.x + directionX[i];
            int newY = current.y + directionY[i];

            if (newX >= 0 && newX < rows && newY >= 0 && newY < cols) {
                if (graph[newX][newY] == 0 && !closedList[newX][newY]) {
                    int tentativeG = gScore[current.x][current.y] + 1;

                    if (tentativeG < gScore[newX][newY]) {
                        gScore[newX][newY] = tentativeG;
                        Node neighbor(newX, newY);
                        neighbor.g = tentativeG;
                        neighbor.h = abs(newX - goal.x) + abs(newY - goal.y);
                        neighbor.f = neighbor.g + neighbor.h;
                        cameFrom[newX][newY] = current;
                        openList.push(neighbor);
                    }
                }
            }
        }
    }

    return std::vector<Node>(); // No path found
}

//Gets the relevant node from the world position
Node PathFindingSystem::nodeFromWorldPos(Vector2 pos) {
	//Idek what to put here
	return Node(static_cast<int>(floor(pos.x / TILE_WIDTH)), static_cast<int>(floor(pos.y / TILE_HEIGHT)));
}

//Updates the grid we have to work with
void PathFindingSystem::updateGrid(const GridChangedEvent* event) {
	grid = event->grid;
}