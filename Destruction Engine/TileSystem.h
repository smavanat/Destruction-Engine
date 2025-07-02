#pragma once
#include <SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include <fstream>
#include <stdio.h>
#include <string>
#include "Coordinator.h"
#include "math.h"
#include "BasicComponents.h"
//The dimensions of the level
const int LEVEL_WIDTH = 1280;
const int LEVEL_HEIGHT = 960;

//Tile constants
const int TILE_WIDTH = 80;
const int TILE_HEIGHT = 80;
const int TOTAL_TILES = 192;
const int TOTAL_TILE_SPRITES = 16;

//The different tile sprites
//const int TILE_RED = 0;
//const int TILE_GREEN = 1;
//const int TILE_BLUE = 2;
//const int TILE_CENTER = 3;
//const int TILE_TOP = 4;
//const int TILE_TOPRIGHT = 5;
//const int TILE_RIGHT = 6;
//const int TILE_BOTTOMRIGHT = 7;
//const int TILE_BOTTOM = 8;
//const int TILE_BOTTOMLEFT = 9;
//const int TILE_LEFT = 10;
//const int TILE_TOPLEFT = 11;

const int TILE_00 = 0;
const int TILE_01 = 1;
const int TILE_02 = 2;
const int TILE_03 = 3;
const int TILE_04 = 4;
const int TILE_05 = 5;
const int TILE_06 = 6;
const int TILE_07 = 7;
const int TILE_08 = 8;
const int TILE_09 = 9;
const int TILE_10 = 10;
const int TILE_11 = 11;
const int TILE_12 = 12;
const int TILE_13 = 13;
const int TILE_14 = 14;
const int TILE_15 = 15;

extern Coordinator gCoordinator;
extern SDL_Renderer* gRenderer;

class TileSystem : public System {
public:
	void init();

	//Sets tiles from tile map
	bool setTiles();

	bool loadTileSet();

	void render();
};

struct TileType : public Component<TileType> {
	int type;

	TileType() = default;

	TileType(int t) : type(t) {}
};