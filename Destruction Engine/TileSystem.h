#pragma once
#include <SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include <fstream>
#include <string>
#include "Coordinator.h"
#include "BasicComponents.h"
#include "GridData.h"
#include "Outline.hpp"

extern Coordinator gCoordinator;
extern b2WorldId worldId;

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