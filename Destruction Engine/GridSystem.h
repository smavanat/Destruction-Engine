#pragma once
#include <vector>
#include <queue>
#include <cmath>
#include "Maths.h"
#include "Coordinator.h"
#include "BasicComponents.h"
#include "GridData.h"
extern Coordinator gCoordinator;

//Holds a representation of the grid in digital form. At some point it will be able to update this representation
//after destruction, but I haven't figured that out yet
class GridSystem : public System {
public:
	void init();
	void update(float dt);

	void createTiles();
	//std::vector<int> loadTiles(std::string path);
	void setGrid(std::shared_ptr<GridData> g);
	void updatePathfinding();
	bool tileStatusChanged(Entity e);
	std::vector<std::vector<int>> convertTilesToGrid();
private:
	std::shared_ptr<GridData> grid;
//	int tileWidth = TILE_WIDTH;
//	int tileHeight = TILE_HEIGHT;
//	int gridWidthInTiles = GRID_WIDTH;
//	int gridHeightInTiles = GRID_HEIGHT;
//	Vector2 origin = Vector2(0,0);
//	std::vector<std::vector<int>> grid{ static_cast<size_t>(gridWidthInTiles*2), std::vector<int>(static_cast<size_t>(gridHeightInTiles*2), -1)};
};

//This struct is used for a* pathfinding and nothing else
struct Node {
	int x, y; //Coordinates of the node in the graph
	int f, g, h; //Values used by the A* algorithm
	bool partial; //Whether or not the node is somewhat destroyed
	int* subcells; //The under-makeup of the node

	Node() = default;

	Node(int xPos, int yPos);

	//Node(int xPos, int yPos)

	//Comparison operators for pq
	bool operator > (const Node& other) const;
	bool operator == (const Node& other) const;
};

//Need to provide a hash definition so that Nodes can work with a hashmap. Only want it based on 
//x and y since that is how we define equalities between nodes
namespace std {
	template<>
	struct hash<Node> {
		std::size_t operator()(const Node& node) const {
			return (std::hash<int>()(node.x) ^ (std::hash<int>()(node.y) << 1));
		}
	};
}

//Provides pathfinding functionality
class PathFindingSystem : public System {
public:
	void init();
	void update(float dt);
	void updateGrid(const GridChangedEvent* event);
	void setGrid(std::shared_ptr<GridData> g);
	std::vector<Node> FindPath(Vector2 start, Vector2 goal);
	std::vector<Node> FindPath2(const std::vector<std::vector<int>> graph, Vector2 start, Vector2 goal);
private:
	Node nodeFromWorldPos(Vector2 pos);
	Vector2 nodeToWorldPos(Node n);
	int getDistance(Node a, Node b);
	std::shared_ptr<GridData> grid;
	/*std::vector<std::vector<int>> grid;*/
};