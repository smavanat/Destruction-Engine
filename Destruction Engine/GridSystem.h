#pragma once
#include <vector>
#include <queue>
#include <cmath>
#include <unordered_map>
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
	void setGrid(std::shared_ptr<GridData> g);
	bool tileStatusChanged(Entity e);
	std::shared_ptr<GridData> grid;
};

//Provides pathfinding functionality
class PathFindingSystem : public System {
public:
	void init();
	void update(float dt);
	void setGrid(std::shared_ptr<GridData> g);
	std::shared_ptr<GridData> grid;
};