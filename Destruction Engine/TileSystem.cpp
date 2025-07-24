#include "TileSystem.h"
SDL_FRect gTileClips[TOTAL_TILE_SPRITES];
Entity tileSet;

TileSystem::TileSystem() {
	gTileClips[TILE_00].x = 0;
	gTileClips[TILE_00].y = 0;
	gTileClips[TILE_00].w = TILE_WIDTH;
	gTileClips[TILE_00].h = TILE_HEIGHT;

	gTileClips[TILE_01].x = 80;
	gTileClips[TILE_01].y = 0;
	gTileClips[TILE_01].w = TILE_WIDTH;
	gTileClips[TILE_01].h = TILE_HEIGHT;

	gTileClips[TILE_02].x = 160;
	gTileClips[TILE_02].y = 0;
	gTileClips[TILE_02].w = TILE_WIDTH;
	gTileClips[TILE_02].h = TILE_HEIGHT;

	gTileClips[TILE_03].x = 240;
	gTileClips[TILE_03].y = 0;
	gTileClips[TILE_03].w = TILE_WIDTH;
	gTileClips[TILE_03].h = TILE_HEIGHT;

	gTileClips[TILE_04].x = 0;
	gTileClips[TILE_04].y = 80;
	gTileClips[TILE_04].w = TILE_WIDTH;
	gTileClips[TILE_04].h = TILE_HEIGHT;

	gTileClips[TILE_05].x = 80;
	gTileClips[TILE_05].y = 80;
	gTileClips[TILE_05].w = TILE_WIDTH;
	gTileClips[TILE_05].h = TILE_HEIGHT;

	gTileClips[TILE_06].x = 160;
	gTileClips[TILE_06].y = 80;
	gTileClips[TILE_06].w = TILE_WIDTH;
	gTileClips[TILE_06].h = TILE_HEIGHT;

	gTileClips[TILE_07].x = 240;
	gTileClips[TILE_07].y = 80;
	gTileClips[TILE_07].w = TILE_WIDTH;
	gTileClips[TILE_07].h = TILE_HEIGHT;

	gTileClips[TILE_08].x = 0;
	gTileClips[TILE_08].y = 160;
	gTileClips[TILE_08].w = TILE_WIDTH;
	gTileClips[TILE_08].h = TILE_HEIGHT;

	gTileClips[TILE_09].x = 80;
	gTileClips[TILE_09].y = 160;
	gTileClips[TILE_09].w = TILE_WIDTH;
	gTileClips[TILE_09].h = TILE_HEIGHT;

	gTileClips[TILE_10].x = 160;
	gTileClips[TILE_10].y = 160;
	gTileClips[TILE_10].w = TILE_WIDTH;
	gTileClips[TILE_10].h = TILE_HEIGHT;

	gTileClips[TILE_11].x = 240;
	gTileClips[TILE_11].y = 160;
	gTileClips[TILE_11].w = TILE_WIDTH;
	gTileClips[TILE_11].h = TILE_HEIGHT;

	gTileClips[TILE_12].x = 0;
	gTileClips[TILE_12].y = 240;
	gTileClips[TILE_12].w = TILE_WIDTH;
	gTileClips[TILE_12].h = TILE_HEIGHT;

	gTileClips[TILE_13].x = 80;
	gTileClips[TILE_13].y = 240;
	gTileClips[TILE_13].w = TILE_WIDTH;
	gTileClips[TILE_13].h = TILE_HEIGHT;

	gTileClips[TILE_14].x = 160;
	gTileClips[TILE_14].y = 240;
	gTileClips[TILE_14].w = TILE_WIDTH;
	gTileClips[TILE_14].h = TILE_HEIGHT;

	gTileClips[TILE_15].x = 240;
	gTileClips[TILE_15].y = 240;
	gTileClips[TILE_15].w = TILE_WIDTH;
	gTileClips[TILE_15].h = TILE_HEIGHT;
}

TileSystem::TileSystem(bool isColliding) {
	forColliders = isColliding;
}

void TileSystem :: init() {}

bool TileSystem::setTiles(std::string path) {
	//Success flag
	bool tilesLoaded = true;

	//Tile offsets
	int x = 0, y = 0;

	//Open the map
	//std::ifstream map("assets/lazy.map");
	std::ifstream map(path);

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
				gCoordinator.addComponent(e, Transform(Vector2(x, y), 0));
				gCoordinator.addComponent(e, TileType(tileType));
				if (forColliders) {
					//I guess add a component here?
					Sprite s = gCoordinator.getComponent<Sprite>(tileSet);
					gCoordinator.addComponent(e, duplicateSprite(x, y , 0, &s, gRenderer, &gTileClips[tileType]));

					std::vector<int> points = { 0, (s.height - 1) * s.width, (s.height * s.width) - 1, s.width - 1 };
					b2BodyId tempId = createTexturePolygon(points, s.width, worldId, s);
					gCoordinator.addComponent(e, tempId);
				}
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
		// if (tilesLoaded)
		// {
		// 	gTileClips[TILE_00].x = 0;
		// 	gTileClips[TILE_00].y = 0;
		// 	gTileClips[TILE_00].w = TILE_WIDTH;
		// 	gTileClips[TILE_00].h = TILE_HEIGHT;

		// 	gTileClips[TILE_01].x = 80;
		// 	gTileClips[TILE_01].y = 0;
		// 	gTileClips[TILE_01].w = TILE_WIDTH;
		// 	gTileClips[TILE_01].h = TILE_HEIGHT;

		// 	gTileClips[TILE_02].x = 160;
		// 	gTileClips[TILE_02].y = 0;
		// 	gTileClips[TILE_02].w = TILE_WIDTH;
		// 	gTileClips[TILE_02].h = TILE_HEIGHT;

		// 	gTileClips[TILE_03].x = 240;
		// 	gTileClips[TILE_03].y = 0;
		// 	gTileClips[TILE_03].w = TILE_WIDTH;
		// 	gTileClips[TILE_03].h = TILE_HEIGHT;

		// 	gTileClips[TILE_04].x = 0;
		// 	gTileClips[TILE_04].y = 80;
		// 	gTileClips[TILE_04].w = TILE_WIDTH;
		// 	gTileClips[TILE_04].h = TILE_HEIGHT;

		// 	gTileClips[TILE_05].x = 80;
		// 	gTileClips[TILE_05].y = 80;
		// 	gTileClips[TILE_05].w = TILE_WIDTH;
		// 	gTileClips[TILE_05].h = TILE_HEIGHT;

		// 	gTileClips[TILE_06].x = 160;
		// 	gTileClips[TILE_06].y = 80;
		// 	gTileClips[TILE_06].w = TILE_WIDTH;
		// 	gTileClips[TILE_06].h = TILE_HEIGHT;

		// 	gTileClips[TILE_07].x = 240;
		// 	gTileClips[TILE_07].y = 80;
		// 	gTileClips[TILE_07].w = TILE_WIDTH;
		// 	gTileClips[TILE_07].h = TILE_HEIGHT;

		// 	gTileClips[TILE_08].x = 0;
		// 	gTileClips[TILE_08].y = 160;
		// 	gTileClips[TILE_08].w = TILE_WIDTH;
		// 	gTileClips[TILE_08].h = TILE_HEIGHT;

		// 	gTileClips[TILE_09].x = 80;
		// 	gTileClips[TILE_09].y = 160;
		// 	gTileClips[TILE_09].w = TILE_WIDTH;
		// 	gTileClips[TILE_09].h = TILE_HEIGHT;

		// 	gTileClips[TILE_10].x = 160;
		// 	gTileClips[TILE_10].y = 160;
		// 	gTileClips[TILE_10].w = TILE_WIDTH;
		// 	gTileClips[TILE_10].h = TILE_HEIGHT;

		// 	gTileClips[TILE_11].x = 240;
		// 	gTileClips[TILE_11].y = 160;
		// 	gTileClips[TILE_11].w = TILE_WIDTH;
		// 	gTileClips[TILE_11].h = TILE_HEIGHT;

		// 	gTileClips[TILE_12].x = 0;
		// 	gTileClips[TILE_12].y = 240;
		// 	gTileClips[TILE_12].w = TILE_WIDTH;
		// 	gTileClips[TILE_12].h = TILE_HEIGHT;

		// 	gTileClips[TILE_13].x = 80;
		// 	gTileClips[TILE_13].y = 240;
		// 	gTileClips[TILE_13].w = TILE_WIDTH;
		// 	gTileClips[TILE_13].h = TILE_HEIGHT;

		// 	gTileClips[TILE_14].x = 160;
		// 	gTileClips[TILE_14].y = 240;
		// 	gTileClips[TILE_14].w = TILE_WIDTH;
		// 	gTileClips[TILE_14].h = TILE_HEIGHT;

		// 	gTileClips[TILE_15].x = 240;
		// 	gTileClips[TILE_15].y = 240;
		// 	gTileClips[TILE_15].w = TILE_WIDTH;
		// 	gTileClips[TILE_15].h = TILE_HEIGHT;
		// }
	}

	//Close the file
	map.close();

	//If the map was loaded fine
	return tilesLoaded;
}

bool TileSystem::loadTileSet(std::string tilePath, std::string setPath) {
	//Loading success flag
	bool success = true;
	
	//Initialise the Entity that holds the tileset
	tileSet = gCoordinator.createEntity();
	gCoordinator.addComponent(tileSet, Sprite(NULL, NULL, Vector2(0.0f, 0.0f), 0, 0, 0.0, false));
	Sprite& s = gCoordinator.getComponent<Sprite>(tileSet);

	//Load tile texture
	if (!loadFromFile(s, setPath, gRenderer)) {
		printf("Failed to load tile set texture!\n");
		success = false;
	}

	if (!setTiles(tilePath)) {
		printf("Failed to load tile set!\n");
		success = false;
	}

	return success;
}

void TileSystem::update(float dt) {
	Sprite& s = gCoordinator.getComponent<Sprite>(tileSet);
	for (Entity e : registeredEntities) {
		/*std::cout << "Entity Signature: " << gCoordinator.getSignature(e).mask << "\n";*/
		Transform &t = gCoordinator.getComponent<Transform>(e);
		TileType& tag = gCoordinator.getComponent<TileType>(e);

		SDL_FRect renderQuad = { t.position.x, t.position.y, TILE_WIDTH, TILE_HEIGHT };

		SDL_RenderTexture(gRenderer, s.texture, &gTileClips[tag.type], &renderQuad);
	}
}