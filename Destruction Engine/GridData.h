#pragma once
#include <vector>
#include <memory>
#include<array>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <cmath>
#include <limits.h>
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

//This struct is used for a* pathfinding and nothing else
struct Node {
	int x, y; //Coordinates of the node in the graph
	int f, g, h; //Values used by the A* algorithm
	bool partial; //Whether or not the node is somewhat destroyed

	Node() = default;

	Node(int xPos, int yPos);

	//Comparison operators for pq
	bool operator > (const Node& other) const;
	bool operator == (const Node& other) const;
};

//Need to provide a hash definition so that Nodes can work with a hashmap. Only want it based on 
//x and y since that is how we define equalities between nodes
namespace std {
	template<>
	struct hash<Node> {
		std::size_t operator()(const Node& node) const {
			return (std::hash<int>()(node.x) ^ (std::hash<int>()(node.y) << 1));
		}
	};
}

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
//Determines whether there is a path, standing in the tile, for an agent to move between
//one direction and another
bool isPathBetween(Direction8 from, Direction8 to, std::shared_ptr<GridData> g, int indexAt, int s);
//Returns the path between two positions
std::vector<Node> FindPath(Vector2 start, Vector2 goal, std::shared_ptr<GridData> grid, int size);