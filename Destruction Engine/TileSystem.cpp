#include "TileSystem.h"
SDL_FRect gTileClips[TOTAL_TILE_SPRITES];
Entity tileSet;

void TileSystem :: init() {}

bool TileSystem::setTiles() {
	//Success flag
	bool tilesLoaded = true;

	//Tile offsets
	int x = 0, y = 0;

	//Open the map
	std::ifstream map("assets/lazy.map");

	//If the map couldn't be loaded
	if (map.fail()) {
		printf("Unable to load map file\n");
		tilesLoaded = false;
	}
	else {
		//Initialise the tiles
		for (int i = 0; i < TOTAL_TILES; i++) {
			//Determines what kind of tile will be made
			int tileType = -1;

			//Read the tile from a map file
			map >> tileType;

			//If there was a problem in reading the map 
			if (map.fail()) {
				//Stop loading the map 
				printf("Error loading the map: Unexpected end of file!\n");
				tilesLoaded = false;
				break;
			}

			//If the number is a valid tile number
			if ((tileType >= 0) && (tileType < TOTAL_TILE_SPRITES)) {
				//Make a new tile
				Entity e = gCoordinator.createEntity();
				gCoordinator.addComponent(e, Tag(0));
				gCoordinator.addComponent(e, Transform(newVector2(x, y), 0));
				gCoordinator.addComponent(e, TileType(tileType));
			}
			else {
				//Stop loading the map 
				printf("Error loading map: Invalid tile type\n");
				tilesLoaded = false;
				break;
			}

			//Move to the next tile spot
			x += TILE_WIDTH;

			//If we have gone too far
			if (x >= LEVEL_WIDTH) {
				//Move back 
				x = 0;

				//Move to the next row
				y += TILE_HEIGHT;
			}
		}

		//Clip the spriteSheet
		if (tilesLoaded)
		{
			gTileClips[TILE_RED].x = 0;
			gTileClips[TILE_RED].y = 0;
			gTileClips[TILE_RED].w = TILE_WIDTH;
			gTileClips[TILE_RED].h = TILE_HEIGHT;

			gTileClips[TILE_GREEN].x = 0;
			gTileClips[TILE_GREEN].y = 80;
			gTileClips[TILE_GREEN].w = TILE_WIDTH;
			gTileClips[TILE_GREEN].h = TILE_HEIGHT;

			gTileClips[TILE_BLUE].x = 0;
			gTileClips[TILE_BLUE].y = 160;
			gTileClips[TILE_BLUE].w = TILE_WIDTH;
			gTileClips[TILE_BLUE].h = TILE_HEIGHT;

			gTileClips[TILE_TOPLEFT].x = 80;
			gTileClips[TILE_TOPLEFT].y = 0;
			gTileClips[TILE_TOPLEFT].w = TILE_WIDTH;
			gTileClips[TILE_TOPLEFT].h = TILE_HEIGHT;

			gTileClips[TILE_LEFT].x = 80;
			gTileClips[TILE_LEFT].y = 80;
			gTileClips[TILE_LEFT].w = TILE_WIDTH;
			gTileClips[TILE_LEFT].h = TILE_HEIGHT;

			gTileClips[TILE_BOTTOMLEFT].x = 80;
			gTileClips[TILE_BOTTOMLEFT].y = 160;
			gTileClips[TILE_BOTTOMLEFT].w = TILE_WIDTH;
			gTileClips[TILE_BOTTOMLEFT].h = TILE_HEIGHT;

			gTileClips[TILE_TOP].x = 160;
			gTileClips[TILE_TOP].y = 0;
			gTileClips[TILE_TOP].w = TILE_WIDTH;
			gTileClips[TILE_TOP].h = TILE_HEIGHT;

			gTileClips[TILE_CENTER].x = 160;
			gTileClips[TILE_CENTER].y = 80;
			gTileClips[TILE_CENTER].w = TILE_WIDTH;
			gTileClips[TILE_CENTER].h = TILE_HEIGHT;

			gTileClips[TILE_BOTTOM].x = 160;
			gTileClips[TILE_BOTTOM].y = 160;
			gTileClips[TILE_BOTTOM].w = TILE_WIDTH;
			gTileClips[TILE_BOTTOM].h = TILE_HEIGHT;

			gTileClips[TILE_TOPRIGHT].x = 240;
			gTileClips[TILE_TOPRIGHT].y = 0;
			gTileClips[TILE_TOPRIGHT].w = TILE_WIDTH;
			gTileClips[TILE_TOPRIGHT].h = TILE_HEIGHT;

			gTileClips[TILE_RIGHT].x = 240;
			gTileClips[TILE_RIGHT].y = 80;
			gTileClips[TILE_RIGHT].w = TILE_WIDTH;
			gTileClips[TILE_RIGHT].h = TILE_HEIGHT;

			gTileClips[TILE_BOTTOMRIGHT].x = 240;
			gTileClips[TILE_BOTTOMRIGHT].y = 160;
			gTileClips[TILE_BOTTOMRIGHT].w = TILE_WIDTH;
			gTileClips[TILE_BOTTOMRIGHT].h = TILE_HEIGHT;
		}
	}

	//Close the file
	map.close();

	//If the map was loaded fine
	return tilesLoaded;
}

bool TileSystem::loadTileSet() {
	//Loading success flag
	bool success = true;
	
	//Initialise the Entity that holds the tileset
	tileSet = gCoordinator.createEntity();
	gCoordinator.addComponent(tileSet, Sprite(NULL, NULL, newVector2(0.0f, 0.0f), 0, 0, 0.0, false));
	Sprite& s = gCoordinator.getComponent<Sprite>(tileSet);

	//Load tile texture
	if (!s_loadFromFile(s, "assets/tiles.png", gRenderer)) {
		printf("Failed to load tile set texture!\n");
		success = false;
	}

	if (!setTiles()) {
		printf("Failed to load tile set!\n");
		success = false;
	}

	return success;
}

void TileSystem::render() {
	Sprite& s = gCoordinator.getComponent<Sprite>(tileSet);
	for (Entity e : registeredEntities) {
		Transform &t = gCoordinator.getComponent<Transform>(e);
		TileType& tag = gCoordinator.getComponent<TileType>(e);

		SDL_FRect renderQuad = { t.position.x, t.position.y, TILE_WIDTH, TILE_HEIGHT };

		SDL_RenderTexture(gRenderer, s.texture, &gTileClips[tag.type], &renderQuad);
	}
}