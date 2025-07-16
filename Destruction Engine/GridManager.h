#pragma once
//#include "TileSystem.h"
#include "GridSystem.h"
#include <fstream>
#include <memory>
class GridSystemManager {
public:
	GridSystemManager();
	GridSystemManager(int tWidth, int tHeight, int gWidth, int gHeight);
	GridSystemManager(int tWidth, int tHeight, int gWidth, int gHeight, std::string path);
	void update(float dt);
	bool loadGridFromFile(std::string path);
private:
	std::shared_ptr<GridSystem> gSystem;
	std::shared_ptr<PathFindingSystem> pSystem;
	std::shared_ptr<GridData> grid;
};

