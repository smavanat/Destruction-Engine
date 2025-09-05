#pragma once
#include <SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include <fstream>
#include <string>
#include "Coordinator.h"
#include "BasicComponents.h"
#include "GridData.h"

extern Coordinator gCoordinator;
extern b2WorldId worldId;
extern SDL_Renderer* gRenderer;

struct TileClip {
	SDL_FRect dimensions;
};

struct TileSet {
	Sprite* srcTex = NULL;
	std::vector<TileClip*> tileClips = std::vector<TileClip*>();
	std::vector<TileClip*> collidingTileClips = std::vector<TileClip*>();
};

//Holds a record of all of the indexes in the collider component array that belong to colliders
//that are impassible terrain
struct TerrainSet {
	uint32_t* eArr; //Holds all the entity ids of the terrain tiles
	uint32_t size; //Holds the number of entities currently in this set
	uint32_t maxsize; //Holds the max number of entities in this set
};

int createNewTileClip(TileSet& t, SDL_FRect d, bool colliding);
bool loadTileMapFromFile(TileSet& t, SDL_Renderer* gRenderer, std::string path, TerrainSet* tSet);
bool initialiseDemoTileMap(TileSet& t, SDL_Renderer* gRenderer, std::string tpath, std::string mpath, TerrainSet* tSet);
void freeTileSet(TileSet& t);

class TileRenderSystem : public System {
public:
	void init() {}

	void update(float dt) {
		//printf("Number of registered entities: %i\n", registeredEntities.size());
		for (Entity entity : registeredEntities) {
			Transform transform = gCoordinator.getComponent<Transform>(entity);
			TileSprite sprite = gCoordinator.getComponent<TileSprite>(entity);

			renderPart(sprite, transform, gRenderer);
		}
	}
};