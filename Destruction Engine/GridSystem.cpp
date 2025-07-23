#include "GridSystem.h"
#include <algorithm>
#include <unordered_set>

const std::unordered_map<Vector2, Direction8>& getDirectionMap() {
    std::unordered_map<Vector2, Direction8> directionMap = {{Vector2(-1, -1), NW}, {Vector2(0, -1), N},
                                                            {Vector2(1, -1), NE},  {Vector2(-1, 0), E},
                                                            {Vector2(1, 0), SE},   {Vector2(-1, 1), S}, 
                                                            {Vector2(0, 1), SW},   {Vector2(1, 1), W}};
    return directionMap;
}

//Initialises the signature for the grid system and creates the tiles for the grid
void GridSystem::init() {
    //Create the tiles
	createTiles();
}

//Checks if the grid has changed in a meaningful way (walkable status of tiles), and if it has, publishes an
//event so the pathfinding system can be updated too
void GridSystem::update(float dt) {
	bool gridChanged = false;
	for (Entity e : registeredEntities) {
		if (tileStatusChanged(e)) {
			gridChanged = true;
		}
	}

	if (gridChanged) {
		//gCoordinator.getEventBus()->publish(new GridChangedEvent(convertTilesToGrid()));
	}
}

//Creates the tiles for the grid
void GridSystem::createTiles() {
    for (int i = 0; i < grid->gridHeight; i++) {
        for (int j = 0; j < grid->gridWidth; j++) {
            Entity e = gCoordinator.createEntity();
            gCoordinator.addComponent(e, Transform(Vector2((j * TILE_WIDTH) + (TILE_WIDTH / 2), ( i* TILE_HEIGHT) + (TILE_HEIGHT / 2)), 0));
            gCoordinator.addComponent(e, Walkable(grid->tiles[toIndex(grid, Vector2(j, i))].status));
        }
	}
}

void GridSystem::setGrid(std::shared_ptr<GridData> g) {
    grid = g;
}

//This only needs to be filled once the actual destruction pathfinding system has been implemented
bool GridSystem::tileStatusChanged(Entity e) {
	return false;
}

//Converts the tiles into their marching squares representation as a 2D vector so we can do pathfinding on them
std::vector<std::vector<int>> GridSystem::convertTilesToGrid() {
    //std::vector<std::vector<int>> tempGrid( static_cast<size_t>(gridWidthInTiles * 2), std::vector<int>(static_cast<size_t>(gridHeightInTiles * 2), -1) );
    std::vector<std::vector<int>> tempGrid(static_cast<size_t>(grid->gridWidth * 2), std::vector<int>(static_cast<size_t>(grid->gridHeight * 2), -1));
    for (Entity e : registeredEntities) {
        Transform t = gCoordinator.getComponent<Transform>(e);
        Walkable w = gCoordinator.getComponent<Walkable>(e);

        //Origin of each set of four is in the top left
        int topX = static_cast<int>(floor(t.position.x / TILE_WIDTH))*2;
        int topY = static_cast<int>(floor(t.position.y / TILE_HEIGHT))*2;

        //Manual conversion of the a* tiles to marching squares representation in the grid for proper pathfinding
        switch (w.walkStatus) {
            case 0:
                tempGrid[topX][topY] = 0;
                tempGrid[topX+1][topY] = 0;
                tempGrid[topX][topY+1] = 0;
                tempGrid[topX+1][topY+1] = 0;
            break;
            case 1:
                tempGrid[topX][topY] = 0;
                tempGrid[topX + 1][topY] = 0;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 0;
            break;
            case 2:
                tempGrid[topX][topY] = 0;
                tempGrid[topX + 1][topY] = 0;
                tempGrid[topX][topY + 1] = 0;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
            case 3:
                tempGrid[topX][topY] = 0;
                tempGrid[topX + 1][topY] = 0;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
            case 4:
                tempGrid[topX][topY] = 0;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 0;
                tempGrid[topX + 1][topY + 1] = 0;
            break;
            case 5:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 0;
                tempGrid[topX][topY + 1] = 0;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
            case 6:
                tempGrid[topX][topY] = 0;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 0;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
            case 7:
                tempGrid[topX][topY] = 0;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
            case 8:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 0;
                tempGrid[topX][topY + 1] = 0;
                tempGrid[topX + 1][topY + 1] = 0;
            break;
            case 9:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 0;
                tempGrid[topX + 1][topY + 1] = 0;
            break;
            case 10:
                tempGrid[topX][topY] = 0;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 0;
            break;
            case 11:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 0;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
            case 12:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 0;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 0;
            break;
            case 13:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 0;
            break;
            case 14:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 0;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
            case 15:
                tempGrid[topX][topY] = 1;
                tempGrid[topX + 1][topY] = 1;
                tempGrid[topX][topY + 1] = 1;
                tempGrid[topX + 1][topY + 1] = 1;
            break;
        }
    }

    /*for (int j = 0; j < tempGrid.size(); j++) {
        for (int i = 0; i < tempGrid[j].size(); i++) {
            printf("%i ", tempGrid[j][i]);
        }
        printf("\n");
    }*/

    return tempGrid;
}

//Pathfinding system initialisation. Incomplete
void PathFindingSystem::init() {

}

void PathFindingSystem::update(float dt) {
    //Need to find a better way of doing this. This is just a quick version that works for now. Use a queue maybe?
    //And then have a function that adds PathFinding requests Entities to the queue or something
    for (Entity e : registeredEntities) {
        Pathfinding &p = gCoordinator.getComponent<Pathfinding>(e);

        //If the component does not have a well defined start and end pos, continue
        if (p.startPos == Vector2() || p.endPos == Vector2()) {
            continue;
        }
        else {
            //This code also appends end and startPos to the list:
            //Probably need to use rdp on it to smooth out the path
            /*
                vecPath.reserve(path.size() + 2);  // Optional optimization
                
                // Add start position
                vecPath.push_back(p.startPos);

                // Convert internal path to world coordinates
                std::transform(path.begin(), path.end(), std::back_inserter(vecPath),
                    [this](const Node& n) { return nodeToWorldPos(n); });

                // Add end position
                vecPath.push_back(p.endPos);
            */

            //Get the path in node form
            auto path = FindPath(p.startPos, p.endPos, grid, 2);
            std::vector<Vector2> vecPath(path.size());
            int cWidth = grid->subWidth;

            //Convert it to coordinates
            std::transform(path.begin(), path.end(), vecPath.begin(),
                [cWidth](Node n) {return nodeToWorldPos(n, cWidth); });

            for (Vector2 v : vecPath) {
                printf("(%f, %f)", v.x, v.y);
            }

            //Set the path value in the component
            p.path = vecPath;
            //Reset the start and endpos to default values to avoid repeat computations
            p.startPos = Vector2();
            p.endPos = Vector2();
        }
    }
}

void PathFindingSystem::setGrid(std::shared_ptr<GridData> g) {
    grid = g;
}