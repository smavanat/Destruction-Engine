#pragma once
#include <vector>
#include <queue>
#include <cmath>
#include "Maths.h"
#include "Coordinator.h"
#include "BasicComponents.h"
extern Coordinator gCoordinator;

class GridSystem : public System {
public:
	void init();
	void update(float dt);

	void setTileWidth(int w);
	void setTileHeight(int h);
	void setGridWidth(int w);
	void setGridHeight(int h);
	void setGridOrigin(Vector2 o);

	int getTileWidth();
	int getTileHeight();
	int getGridWidth();
	int getGridHeight();
	Vector2 getGridOrigin();

	void createTiles();
	bool tileStatusChanged(Entity e);
	std::vector<int> convertTilesToGrid();
private:
	int tileWidth = 32;
	int tileHeight = 32;
	int gridWidthInTiles = 32;
	int gridHeightInTiles = 32;
	Vector2 origin = newVector2(0,0);
};

struct Grid : public Component<Grid> {
	Vector2 origin;
	int tileWidth;
	int tileHeight;
	int gridWidthInTiles;
	int gridHeightInTiles;
};

struct Walkable : public Component<Walkable> {
	bool walkable;
};

struct Node {
	int x, y; //Coordinates of the node in the graph
	int f, g, h; //Values used by the A* algorithm

	Node(int xPos, int yPos);

	//Comparison operators for pq
	bool operator > (const Node& other) const;
	bool operator == (const Node& other) const;
};

class PathFindingSystem : public System {
public:
	void init();
	void update(float dt);
	std::vector<Node> FindPath(const std::vector<std::vector<int>> graph, Vector2 start, Vector2 goal);
	Node nodeFromWorldPos(Vector2 pos);
	void updateGrid(const GridChangedEvent* event);
private:
	std::vector<int> grid;
};