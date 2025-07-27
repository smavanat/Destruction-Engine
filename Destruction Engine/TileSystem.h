#pragma once
#include <SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include <fstream>
#include <string>
#include "Coordinator.h"
#include "BasicComponents.h"
#include "GridData.h"
#include "Outline.hpp"

// const int TILE_00 = 0;
// const int TILE_01 = 1;
// const int TILE_02 = 2;
// const int TILE_03 = 3;
// const int TILE_04 = 4;
// const int TILE_05 = 5;
// const int TILE_06 = 6;
// const int TILE_07 = 7;
// const int TILE_08 = 8;
// const int TILE_09 = 9;
// const int TILE_10 = 10;
// const int TILE_11 = 11;
// const int TILE_12 = 12;
// const int TILE_13 = 13;
// const int TILE_14 = 14;
// const int TILE_15 = 15;

extern Coordinator gCoordinator;
//extern SDL_Renderer* gRenderer;
extern b2WorldId worldId;

// class TileSystem : public System {
// public:
// 	void init();

// 	//Sets tiles from tile map
// 	bool setTiles(std::string path);

// 	bool loadTileSet(std::string tilePath, std::string setPath);

// 	void update(float dt);

// 	bool forColliders;
// };


struct TileClip {
	SDL_FRect dimensions;
};

struct TileSet {
	Sprite* srcTex = NULL;
	std::vector<TileClip*> tileClips = std::vector<TileClip*>();
	std::vector<TileClip*> collidingTileClips = std::vector<TileClip*>();
};

SDL_Texture* loadTextureFromFile(SDL_Renderer* gRenderer, std::string path);
int createNewTileClip(TileSet& t, SDL_FRect d, bool colliding);
bool loadTileMapFromFile(TileSet& t, SDL_Renderer* gRenderer, std::string path);
bool initialiseDemoTileMap(TileSet& t, SDL_Renderer* gRenderer, std::string tpath, std::string mpath);
void freeTileSet(TileSet& t);