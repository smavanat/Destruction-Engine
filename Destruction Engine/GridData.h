#pragma once
#include <vector>
#include <memory>
#include<array>
#include <algorithm>
#include "Maths.h"

//For now, just have these be constants. Need to figure out how to link grid and pathfinding properly
//at a later date
const int GRID_HEIGHT = 12;
const int GRID_WIDTH = 16;

//The dimensions of the level
const int LEVEL_WIDTH = 1280;
const int LEVEL_HEIGHT = 960;

//Tile constants
const int TILE_WIDTH = 80;
const int TILE_HEIGHT = 80;
const int TOTAL_TILES = 192;
const int TOTAL_TILE_SPRITES = 16;

struct TileData {
	uint8_t status; //0 -> walkable, 1 -> blocked 2 -> partial
	uint8_t type; //Represents the type of tile, could be used for weighted pathfinding
	
	std::vector<int> subcells; //Holds the subcell representation of the grid
	std::array<bool, 4> exitable; //Holds which sides can be exited from in this tile
};

struct GridData {
	Vector2 origin = Vector2(0, 0);
	int subWidth = 4; //Holds the width of the tiles (all tiles should be squares so this is enough)
	int tileWidth = TILE_WIDTH; //Tile width in pixels
	int tileHeight = TILE_HEIGHT; //Tile height in pixels
	int gridWidth = GRID_WIDTH; //Width of the grid in tiles
	int gridHeight = GRID_HEIGHT; //Height of the grid in tiles
	std::vector<TileData> tiles;
};

enum Direction8 {
	NW,
	N,
	NE,
	E,
	SE,
	S,
	SW,
	W,
	INVALID
};

enum Direction4 {
	N_4, E_4, W_4, S_4
};

//Global methods that need to be used by both Path and Grid systems

//Gets the grid position from a world position
Vector2 gridToWorldPos(std::shared_ptr<GridData> g, Vector2 gridPos);
//Gets the world position from a grid position
Vector2 worldToGridPos(std::shared_ptr<GridData> g, Vector2 worldPos);
//Converts a grid position to a 1D array index
int toIndex(std::shared_ptr<GridData> g, Vector2 gridPos);
//Checks if a grid position is in bounds
bool inBounds(std::shared_ptr<GridData> g, Vector2 gridPos);
//Determines the number of exitable sides that a tile has
int numExits(const TileData& t);
//Determines whether an agent of a certain size could walk through this tile
bool isPathable(const TileData& t, Direction8 d, int s, int w);
//Determines whether a path exists for an agent of a certain size through the subcell grid
//For processing pathfinding across adjacent tiles
bool isPathableWithAdjacent(int index, std::shared_ptr<GridData> g, Direction8 d, int s);
//Determines whether there is a path, standing in the tile, for an agent to move between
//one direction and another
bool isPathBetween(Direction8 from, Direction8 to, std::shared_ptr<GridData> g, int indexAt, int indexTo, int s);

//These functions should not be included in this header file in the final build
//They are here for now so I can run unit tests on them in the testbed
bool isValidPos(std::vector<int> subcellArr, int w, int x, int y, int s); 
std::vector<bool> preprocessValidPositions(std::vector<int> subcellArr, int w, int s); 
bool touchesEdge(int x, int y, int s, int n); 
bool checkEdge(int x, int y, int s, int n, Direction8 d); 
bool pathExists(int startX, int startY, int s, int w, std::vector<bool> pArr, Direction8 startDirection); 
bool pathExistsTo(int startX, int startY, int endX, int endY, int s, int w, std::vector<bool> pArr);
std::pair<int, int> getStartPos(std::vector<int> subcellArr, int w, int s, Direction8 d);
std::vector<int> combineTiles(std::vector<std::vector<int>*> tArray, int w);
std::vector<int> getCombinedSubcellGrid(int index, std::shared_ptr<GridData> g, Direction8 d);
int getWidth(int index, int w, Direction8 d);
bool isAtTopEdge(Vector2 vec);
bool isAtLeftEdge(Vector2 vec);
bool isAtRightEdge(Vector2 vec);
bool isAtBottomEdge(Vector2 vec);
std::vector<Vector2> trimCells(int index, int gridWidth, int gridHeight, Direction8 d);
std::vector<int>* getNeighbourCells(int index, std::shared_ptr<GridData>g, Vector2 vec);
std::vector<int> createSurroundGrid(int index, std::shared_ptr<GridData> g, Direction8 d);
