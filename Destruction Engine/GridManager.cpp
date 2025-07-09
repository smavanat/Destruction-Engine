#include "GridManager.h"
GridSystemManager::GridSystemManager() {
    gSystem = nullptr;
    pSystem = nullptr;
    //tSystem = nullptr;
    grid = std::make_shared<GridData>();

    //Initialising the grid height
    grid->tileWidth = 0;
    grid->tileHeight = 0;
    grid->gridWidth = 0;
    grid->gridHeight = 0;
    grid->tiles = std::vector<TileData>( static_cast<size_t>(grid->gridWidth * grid->gridHeight) );
}

GridSystemManager::GridSystemManager(int tWidth, int tHeight, int gWidth, int gHeight) {
    //This is a really annoying way of having to setup the signatures of Systems, need to find a better way
    /*{
        Signature sig;
        sig.addComponent<Transform>();
        sig.addComponent<TileType>();
        tSystem = gCoordinator.addSystem<TileSystem>(sig);
    }*/
    //tSystem->init(); //Maybe these init calls are not necessary??
    {
        Signature sig;
        sig.addComponent<Transform>();
        sig.addComponent<Walkable>();
        gSystem = gCoordinator.addSystem<GridSystem>(sig);
    }
    //gSystem->init();
    {
        Signature sig;
        sig.addComponent<Pathfinding>();
        pSystem = gCoordinator.addSystem<PathFindingSystem>(sig);
    }
    //pSystem->init();

    grid = std::make_shared<GridData>();

    //Initialising the grid height
    grid->tileWidth = tWidth;
    grid->tileHeight = tHeight;
    grid->gridWidth = gWidth;
    grid->gridHeight = gHeight;
    grid->tiles = std::vector<TileData>( static_cast<size_t>(gWidth * gHeight) );

    //Making sure the child systems all have the same pointer
    gSystem->setGrid(grid);
    pSystem->setGrid(grid);
}

GridSystemManager::GridSystemManager(int tWidth, int tHeight, int gWidth, int gHeight, std::string path) : GridSystemManager(tWidth, tHeight, gWidth, gHeight) {
    loadGridFromFile(path);
}

void GridSystemManager::update(float dt) {
    gSystem->update(dt);
    pSystem->update(dt);
}

bool GridSystemManager::loadGridFromFile(std::string path) {
    //Make sure that the tileset is loaded properly
    //if (!tSystem->loadTileSet(path))
    //    return false;

    //Loading the grid to make the GridData tile vector:
    std::ifstream map(path);

    if (map.fail()) {
        printf("Unable to get grid data.\n");
    }
    else {
        for (int i = 0; i < grid->gridHeight * grid->gridWidth; i++) {
            int tileType = -1;
            map >> tileType;

            //If there was a problem in reading the map 
            if (map.fail()) {
                //Stop loading the map 
                printf("Error loading the map: Unexpected end of file!\n");
                return false;
            }

            //If the number is a valid tile number
            if ((tileType >= 0) && (tileType < TOTAL_TILE_SPRITES)) {
                //Make a new tile
                grid->tiles[i].type = tileType;
                //Empty tile
                if (tileType == 0) {
                    grid->tiles[i].status = 0;
                    grid->tiles[i].subcells.fill(0);
                }
                //Filled tile
                else {
                    grid->tiles[i].status = 1;
                    grid->tiles[i].subcells.fill(1);
                }
            }
            else {
                //Stop loading the map 
                printf("Error loading map: Invalid tile type\n");
                return false;
            }

        }
    }
    return true;
}