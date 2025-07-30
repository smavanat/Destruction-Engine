#pragma once
#include "TileSystem.h"
#include "GridSystem.h"
#include <fstream>
#include <memory>
class GridSystemManager {
public:
	GridSystemManager();
	GridSystemManager(int tWidth, int gWidth, int gHeight);
	GridSystemManager(int tWidth, int gWidth, int gHeight, std::string path);
	void update(float dt);
	bool loadGridFromFile(std::string path);
	bool setGridTileColliders(TerrainSet* tSet);
private:
	std::shared_ptr<GridSystem> gSystem;
	std::shared_ptr<PathFindingSystem> pSystem;
	std::shared_ptr<GridData> grid;
};

bool isOverlapping(SDL_FRect* t, Collider* c);