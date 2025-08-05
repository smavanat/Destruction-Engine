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

//Provides pathfinding functionality
class PathFindingSystem : public System {
public:
	void init();
	void update(float dt);
	void setGrid(std::shared_ptr<GridData> g);
	std::shared_ptr<GridData> grid;
};