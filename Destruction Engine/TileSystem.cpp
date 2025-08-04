#include "TileSystem.h"
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
	TileClip* tc = (TileClip*)malloc(sizeof(TileClip));
	*tc = (TileClip){d};
	if (colliding) t.collidingTileClips.push_back(tc);
	else t.tileClips.push_back(tc);
	return 0;
}

int addElementToTerrainSet(TerrainSet* tSet, Collider* c) {
	printf("Collider Pointer before adding: %i\n", c);
	if(tSet->size >= tSet->maxsize) {
		Collider** temp = (Collider**)malloc(tSet->maxsize*2*sizeof(Collider*));
		memcpy(temp, tSet->cArr, tSet->maxsize*sizeof(Collider*));
		tSet->cArr = temp;
		tSet->maxsize*=2;
	}

	tSet->cArr[tSet->size] = c;
	tSet->size++;
	return 0;
}

bool loadTileMapFromFile(TileSet& t, SDL_Renderer* gRenderer, std::string path, TerrainSet* tSet) {
	//Success flag
	bool tilesLoaded = true;

	//Tile offsets
	int x = TILE_WIDTH/2, y = TILE_HEIGHT/2;

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

			//If the number is a non-colliding tile 
			if ((tileType >= 0) && (tileType < t.tileClips.size())) {
				//Make a new tile
				Entity e = gCoordinator.createEntity();
				Transform tr = Transform((Vector2){x, y}, 0);
				gCoordinator.addComponent(e, tr);

				//If the sprite is non-collidable, we can just set its texture to be the texture of the 
				//source sprite and actually give it a srcRect, and then change its renderfunc pointer to go to renderPart
				TileSprite s = TileSprite(t.srcTex->texture, &t.tileClips[tileType]->dimensions);
				gCoordinator.addComponent(e, s);
			}
			//If this is a tile that interacts with the destruction system, we need to give
			//it its own sprite, otherwise we won't be able to modify the pixels
			else if ((tileType >= t.tileClips.size()) && (tileType < t.collidingTileClips.size()+t.tileClips.size())) {
				//Make a new tile
				Entity e = gCoordinator.createEntity();
				Transform tr = Transform((Vector2){x, y}, 0);
				gCoordinator.addComponent(e, tr);
				
				int index = tileType - t.tileClips.size();
				//We need to duplicate the part of the source sprite that we want so we can get
				//the actual pixel data.
				Sprite s = duplicateSprite(t.srcTex, gRenderer, &t.collidingTileClips[index]->dimensions);
				gCoordinator.addComponent(e, s);
				//Adding the collider here.
				std::vector<int> points = { 0, (s.surfacePixels->h - 1) * s.surfacePixels->w, (s.surfacePixels->h * s.surfacePixels->w) - 1, s.surfacePixels->w - 1 };
				b2BodyId tempId = createTexturePolygon(points, static_cast<int>(t.collidingTileClips[index]->dimensions.w), worldId, s, tr);
				Collider* c = new Collider(tempId);
				gCoordinator.addComponent(e, *c);
				addElementToTerrainSet(tSet, c);
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
				x = TILE_WIDTH/2;

				//Move to the next row
				y += TILE_HEIGHT;
			}
		}
		printf("Printing Collider Pointers in function\n");
		for(int i = 0; i < tSet->size; i++) {
			printf("%i\n", tSet->cArr[i]);
		}
	}

	//Close the file
	map.close();

	//If the map was loaded fine
	return tilesLoaded;
}

//Need to fix this function so that we are not crating the Tilset locally on the stack, and instead passing it 
//in from the heap.
bool initialiseDemoTileMap(TileSet& t, SDL_Renderer* gRenderer, std::string tpath, std::string mpath, TerrainSet* tSet) {
	loadFromFile(*t.srcTex, tpath, gRenderer);
	if(t.srcTex->texture == NULL){
		std::cout << "Failed to load TileSet source Texture!\n" << SDL_GetError();
		return false;
	}

	SDL_FRect blankTile = (SDL_FRect){0, 0, TILE_WIDTH, TILE_HEIGHT};
	SDL_FRect filledTile = (SDL_FRect){240, 240, TILE_WIDTH, TILE_HEIGHT};

	createNewTileClip(t, blankTile, false);
	createNewTileClip(t, filledTile, true);

	if(!loadTileMapFromFile(t, gRenderer, mpath, tSet)) {
		std::cout << "Failed to load TileSet map data!\n" << SDL_GetError();
		return false;
	} 

	return true;
}

void freeTileSet(TileSet& t) {
	for(TileClip* tc : t.tileClips){
		free(tc);
	}

	for(TileClip* tc : t.collidingTileClips) {
		free(tc);
	}

	free(t.srcTex);
}
