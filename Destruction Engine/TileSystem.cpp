#include "TileSystem.h"
SDL_FRect gTileClips[TOTAL_TILE_SPRITES];
Entity tileSet;

void TileSystem :: init() {
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
				Transform t = Transform(Vector2(x, y), 0);
				gCoordinator.addComponent(e, t);
				gCoordinator.addComponent(e, TileType(tileType));
				if (forColliders) {
					//I guess add a component here?
					Sprite s = gCoordinator.getComponent<Sprite>(tileSet);
					gCoordinator.addComponent(e, duplicateSprite(&s, gRenderer, &gTileClips[tileType]));

					std::vector<int> points = { 0, (s.height - 1) * s.width, (s.height * s.width) - 1, s.width - 1 };
					b2BodyId tempId = createTexturePolygon(points, s.width, worldId, s, t);
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
	gCoordinator.addComponent(tileSet, Sprite(NULL, NULL, 0, 0, false));
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

//Loads a texture from a specified path and returns a pointer to it so we don't keep copying it
SDL_Texture* loadTextureFromFile(SDL_Renderer* gRenderer, std::string path) {
	SDL_Texture* loaded = NULL;
	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if(loadedSurface == NULL) {
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), SDL_GetError());
	}
	else {
		//Create texture from surface pixels:
		loaded = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if(loaded == NULL) {
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		//Get rid of the old loaded surface
		SDL_DestroySurface(loadedSurface);
	}
	return loaded;
}

int createNewTileClip(TileSet& t, SDL_FRect d, bool colliding) {
	TileClip tc = (TileClip){d, colliding};
	t.tileClips.push_back(&tc);
	return 0;
}

bool loadTileMapFromFile(TileSet& t, SDL_Renderer* gRenderer, std::string path) {
	//Success flag
	bool tilesLoaded = true;

	//Tile offsets
	int x = 0, y = 0;

	//Open the map
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
			if ((tileType >= 0) && (tileType < t.tileClips.size())) {
				//Make a new tile
				Entity e = gCoordinator.createEntity();
				Transform tr = Transform(Vector2(x, y), 0);
				gCoordinator.addComponent(e, tr);
				//gCoordinator.addComponent(e, TileType(tileType));

				//If this is a tile that interacts with the destruction system, we need to give
				//it its own sprite, otherwise we won't be able to modify the pixels
				if (t.tileClips[tileType]->colliding) {
					Sprite s = duplicateSprite(t.srcTex, gRenderer, &gTileClips[tileType]);
					gCoordinator.addComponent(e, s);

					//Adding the collider here.
					std::vector<int> points = { 0, (s.height - 1) * s.width, (s.height * s.width) - 1, s.width - 1 };
					b2BodyId tempId = createTexturePolygon(points, static_cast<int>(t.tileClips[tileType]->dimensions.w), worldId, s, tr);
					gCoordinator.addComponent(e, tempId);
				}
				else {
					Sprite s = gCoordinator.getComponent<Sprite>(tileSet);
					gCoordinator.addComponent(e, duplicateSprite(&s, gRenderer, &gTileClips[tileType]));
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
	}

	//Close the file
	map.close();

	//If the map was loaded fine
	return tilesLoaded;
}

int initialiseDemoTileMap(TileSet& t, SDL_Renderer* gRenderer, std::string tpath, std::string mpath) {
	return 0;
}