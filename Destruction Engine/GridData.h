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
	std::array<uint8_t, 16> subcells;

	/*TileData() = default;

	TileData(uint8_t s, std::array<uint8_t, 16> subcells;) : status(s) {
		std::copy(sArr, sArr + 16, subcells);
	}*/
};

struct GridData {
	Vector2 origin = Vector2(0, 0);
	int tileWidth = TILE_WIDTH;
	int tileHeight = TILE_HEIGHT;
	int gridWidth = GRID_WIDTH;
	int gridHeight = GRID_HEIGHT;
	std::vector<TileData> tiles;
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