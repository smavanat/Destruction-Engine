#include "GridSystem.h"
#include <algorithm>

void GridSystem::init() {
	Signature sig;
	sig.addComponent<Transform>();
	sig.addComponent<Walkable>();
	gCoordinator.setSystemSignature<GridSystem>(sig);
	createTiles();
}

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

void GridSystem::createTiles() {
	for (int i = 0; i < gridHeightInTiles * gridWidthInTiles; i++) {
		Entity e = gCoordinator.createEntity();
		//How do we determine whether a tile is walkable or not initially?
	}
}

//This only needs to be filled once the actual destruction pathfinding system has been implemented
bool GridSystem::tileStatusChanged(Entity e) {
	return false;
}

std::vector<int> GridSystem::convertTilesToGrid() {

}

Node::Node(int xPos, int yPos) : x(xPos), y(yPos), f(0), g(0), h(0) {}

bool Node::operator>(const Node& other) const {
	return f > other.f;
}

bool Node::operator==(const Node& other) const {
	return x == other.x && y == other.y;
}

void PathFindingSystem::init() {
	gCoordinator.getEventBus()->subscribe(this, &PathFindingSystem::updateGrid);
}

void PathFindingSystem::update(float dt) {

}

std::vector<Node> PathFindingSystem::FindPath(const std::vector<std::vector<int>> graph, Vector2 startPos, Vector2 goalPos) {
	//Node versions of start and end pos
	Node start = nodeFromWorldPos(startPos);
	Node goal = nodeFromWorldPos(goalPos);
	//Coordinates of four possible movement directions. Should expand to 8 later
	const int directionX[] = { -1, 0, 1, 0 };
	const int directionY[] = { 0, 1, 0, -1 };

	//Initialise open and closed lists
	std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openList;
	std::vector<std::vector<bool>> closedList(graph.size(), std::vector<bool>(graph[0].size(), false));

	openList.push(start);

	//Main loop
	while (!openList.empty()) {
		//Get the cell with the lowest f value from the open list
		Node current = openList.top();
		openList.pop();

		//Check if the current cell is the goal
		if (current == goal) {
			//Reconstruct the path
			std::vector<Node> path;
			while (!(current == start)) {
				path.push_back(current);
				current = graph[current.x][current.y];
			}
			path.push_back(start);
			reverse(path.begin(), path.end());
			return path;
		}

		closedList[current.x][current.y] = true;
		for (int i = 0; i < 4; i++) {
			int newX = current.x + directionX[i];
			int newY = current.y + directionY[i];

			//Check that the neighbour is in the grid boundaries
			if (newX >= 0 && newX < graph.size() && newY >= 0 && newY < graph[0].size()) {
				//Check if the neighbour is walkable and not in the closed list
				if (graph[newX][newY] == 0 && !closedList[newX][newY]) {
					Node neighbour(newX, newY);

					int newG = current.g + 1;

					//Check if the neighbour is not in the open list or has a lower g value
					if (newG < neighbour.g || !closedList[newX][newY]) {
						neighbour.g = newG;
						neighbour.h = abs(newX - goal.x) + abs(newY - goal.y);
						neighbour.f = neighbour.g + neighbour.h;
						graph[newX][newY] = current; //Update the parent of the neighbour
						openList.push(neighbour); //Add the neighbour to the open list
					}
				}
			}
		}
	}
	//No path found 
	return std::vector<Node>();
}

Node PathFindingSystem::nodeFromWorldPos(Vector2 pos) {
	//Idek what to put here
}

//Updates the grid we have to work with
void PathFindingSystem::updateGrid(const GridChangedEvent* event) {
	grid = event->grid;
}