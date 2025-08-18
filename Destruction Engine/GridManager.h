#pragma once
#include "Maths.h"
#include "TileSystem.h"
#include "PathSystem.h"
#include <fstream>
#include <memory>
#include <stdint.h>
class GridSystemManager {
public:
	GridSystemManager();
	GridSystemManager(int tWidth, int gWidth, int gHeight);
	GridSystemManager(int tWidth, int gWidth, int gHeight, std::string path);
	void update(float dt);
	bool loadGridFromFile(std::string path);
	void setGridTileColliders(TerrainSet* tSet);
	void printWorldGrid();
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

typedef struct {
	Vector2 a;
	Vector2 b;
} Edge;

typedef struct {
	Edge* points;
	uint32_t* count;
	uint32_t size;
	uint32_t maxSize;
} CountMap;

typedef struct {
	Vector2* forward;
	Vector2* back;
	uint32_t size;
	uint32_t maxSize;
} PointMap;

typedef struct {
	Vector2* entries;
	uint32_t size;
	uint32_t maxSize;
} VectorList;

typedef struct {
    Vector2 ul;
    Vector2 ur;
    Vector2 ll;
    Vector2 lr;
} Quad;

bool isOverlapping(SDL_FRect* t, Collider* c);
bool isOverlapping(SDL_FRect* t1, float r1, SDL_FRect* t2, float r2);
void intersectingSubcells(std::shared_ptr<GridData> g, Collider* c, bool setUnWalkable, Vector2 start);
