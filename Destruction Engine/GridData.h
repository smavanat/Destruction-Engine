#pragma once
#include <vector>
#include <memory>
#include<array>
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
	std::array<uint8_t, 16> subcells; //Holds the subcell representation of the grid
	std::array<bool, 4> exitable; //Holds which sides can be exited from in this tile
};

struct GridData {
	Vector2 origin = Vector2(0, 0);
	int tileWidth = TILE_WIDTH;
	int tileHeight = TILE_HEIGHT;
	int gridWidth = GRID_WIDTH;
	int gridHeight = GRID_HEIGHT;
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
	W
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
bool isPathable(const TileData& t, Direction8 d, int size);
//Determines whether a path exists for an agent of a certain size through the subcell grid
bool pathExists(int startX, int startY, int s, const TileData& t, const std::array<bool, 16>& pArr, Direction8 startDirection);