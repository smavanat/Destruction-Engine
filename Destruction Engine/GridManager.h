#pragma once
#include "TileSystem.h"
#include "PathSystem.h"
#include <fstream>
#include <memory>
class GridSystemManager {
public:
	GridSystemManager();
	GridSystemManager(int tWidth, int gWidth, int gHeight);
	GridSystemManager(int tWidth, int gWidth, int gHeight, std::string path);
	void update(float dt);
	bool loadGridFromFile(std::string path);
	void setGridTileColliders(TerrainSet* tSet);
	// std::shared_ptr<GridSystem> gSystem;
	std::shared_ptr<PathFindingSystem> pSystem;
	std::shared_ptr<GridData> grid;
};

struct GHVertex {
	GHVertex* next, *prev, *neighbour;
	Vector2 p;
	float alpha; //This represents the distance the intersection is along a polygon edge in the range [0, 1]
	bool intersect; //Whether this point represents an edge intersection
	bool entry; //entry or exit flag (whether travelling along the subject polygon in a given order enters or leaves it at this vertex)
	bool visited;

};

struct SegmentIntersection {
	Vector2 point;
	double tS; //This represents the distance the intersection is along the subject polygon edge in the range [0, 1]
	double tC; //This represents the distance the intersection is along the clip polygon edge in the range [0, 1]
	bool valid;
};

bool isOverlapping(SDL_FRect* t, Collider* c);
void intersectingSubcells(std::shared_ptr<GridData> g, int index, Collider* c, bool setUnWalkable);