#include "GridManager.h"
GridSystemManager::GridSystemManager() {
    gSystem = nullptr;
    pSystem = nullptr;
    //tSystem = nullptr;
    grid = std::make_shared<GridData>();

    //Initialising the grid height
    grid->subWidth = 0;
    grid->tileWidth = 0;
    grid->tileHeight = 0;
    grid->gridWidth = 0;
    grid->gridHeight = 0;
    grid->tiles = std::vector<TileData>( static_cast<size_t>(grid->gridWidth * grid->gridHeight) );
}

GridSystemManager::GridSystemManager(int tWidth, int tHeight, int gWidth, int gHeight) {
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
    grid->subWidth = 4;
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

bool GridSystemManager::setGridTileColliders(TerrainSet* tSet) {
    return true;
}

bool GridSystemManager::loadGridFromFile(std::string path) {
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
                    //grid->tiles[i].subcells.fill(0);
                    grid->tiles[i].subcells = std::vector<int>(grid->subWidth*grid->subWidth, 0);
                    grid->tiles[i].exitable.fill(true);
                }
                //Filled tile
                else {
                    grid->tiles[i].status = 1;
                    //grid->tiles[i].subcells.fill(1);
                    grid->tiles[i].subcells = std::vector<int>(grid->subWidth*grid->subWidth, 1);
                    grid->tiles[i].exitable.fill(false);
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

#pragma region SAT
b2Vec2* getSeperatingAxes(b2ShapeId id) {
    b2Vec2* axes = (b2Vec2*)calloc(3, sizeof(b2Vec2));
    b2Vec2* colliderVertices = b2Shape_GetPolygon(id).vertices;
    for(int i = 0; i < 3; i++) {
        //Get the current vertex
        b2Vec2 start = colliderVertices[i];
        //Get the next vertex
        b2Vec2 end = colliderVertices[(i + 1) % 3];
        //Subtract the two to get the edge vector
        b2Vec2 edge = (b2Vec2){(end.x - start.x), (end.y - start.y)};
        //Get the normal of the edge
        b2Vec2 normal = (b2Vec2){-edge.y, edge.x};
        //Since the coordinates of collider vertices are in box2D's own internal measurements
        //rather than pixels since that would break box2D
        // normal.x *= metresToPixels;
        // normal.y *= metresToPixels;
        float length = sqrt(normal.x * normal.x + normal.y * normal.y);
        if (length > 0) {
            normal.x /= length;
            normal.y /= length;
        }
        axes[i] = normal;
    }
    return axes;
}

b2Vec2* getSeperatingAxes(SDL_FRect* rect) {
    b2Vec2* axes = (b2Vec2*)calloc(4, sizeof(b2Vec2));
    b2Vec2 colliderVertices[] = {(b2Vec2){rect->x, rect->y}, (b2Vec2){rect->x+rect->w, rect->y}, (b2Vec2){rect->x+rect->w, rect->y+rect->h}, (b2Vec2){rect->x, rect->y+rect->h}};

    for(int i = 0; i < 4; i++) {
        //Get the current vertex
        b2Vec2 start = colliderVertices[i];
        //Get the next vertex
        b2Vec2 end = colliderVertices[(i + 1) % 4];
        //Subtract the two to get the edge vector
        b2Vec2 edge = (b2Vec2){end.x - start.x, end.y - start.y};
        //Get the normal of the edge
        b2Vec2 normal = (b2Vec2){-edge.y, edge.x};
        float length = sqrt(normal.x * normal.x + normal.y * normal.y);
        if (length > 0) {
            normal.x /= length;
            normal.y /= length;
        }
        axes[i] = normal;
    }
    return axes;
}

b2Vec2 projectShape(b2ShapeId id, b2Vec2* axis) {
    b2Vec2* colliderVertices = b2Shape_GetPolygon(id).vertices;
    double min = (axis->x * colliderVertices[0].x*metresToPixels) + (axis->y * colliderVertices[0].y*metresToPixels);
    double max = min;

    for(int i = 1; i < 3; i++) {
        double c = (axis->x * colliderVertices[i].x*metresToPixels) + (axis->y * colliderVertices[i].y*metresToPixels);
        if(c < min) min = c;
        else if(c > max) max = c;
    }
    
    return (b2Vec2){min, max}; //No clue why multiplying this by metresToPixels fixes things
}

b2Vec2 projectShape(SDL_FRect* rect, b2Vec2* axis) {
    b2Vec2 colliderVertices[] = {(b2Vec2){rect->x, rect->y}, (b2Vec2){rect->x+rect->w, rect->y}, (b2Vec2){rect->x+rect->w, rect->y+rect->h}, (b2Vec2){rect->x, rect->y+rect->h}};
    double min = (axis->x * colliderVertices[0].x) + (axis->y * colliderVertices[0].y);
    double max = min;

    for(int i = 1; i < 4; i++) {
        double c = (axis->x * colliderVertices[i].x) + (axis->y * colliderVertices[i].y);
        if(c < min) min = c;
        else if(c > max) max = c;
    }
    
    return (b2Vec2){min, max};
}

bool overlap(b2Vec2* a, b2Vec2* b) {
    return a->x <= b->y && b->x <= a->y;
}

bool isOverlapping(SDL_FRect* t, Collider* c) {
    b2Vec2* axes1 = getSeperatingAxes(t);

    int shapeCount = b2Body_GetShapeCount(c->colliderId);
    b2Vec2 colliderPosition = b2Body_GetPosition(c->colliderId);
    b2ShapeId* colliderShapes = (b2ShapeId*)malloc(shapeCount*sizeof(b2ShapeId));
    b2Body_GetShapes(c->colliderId, colliderShapes, shapeCount);

    printf("Angle: %f\n", b2Rot_GetAngle(b2Body_GetRotation(c->colliderId)));

    printf("Collider Vertices:\n");
    for(int i = 0; i < shapeCount; i++) {
        b2Vec2* verts = b2Shape_GetPolygon(colliderShapes[i]).vertices;
        for(int j = 0; j < 3; j++) {
            printf("(%f, %f)\n", (verts[j].x + colliderPosition.x)*metresToPixels, (verts[j].y + colliderPosition.x)*metresToPixels);
        }
    }

    for(int i = 0; i < shapeCount; i++) {
        b2Vec2* axes2 = getSeperatingAxes(colliderShapes[i]);
        bool doesShapeOverlap = true;

        for(int j = 0; j < 4; j++) {
            b2Vec2 p1 = projectShape(t, &axes1[j]);
            printf("rprojX: %f, rprojY: %f\n", p1.x, p1.y);
            b2Vec2 p2 = projectShape(colliderShapes[i], &axes1[j]);
            printf("cprojX: %f, cprojY: %f\n", p2.x, p2.y);
            //If projection does not overlap then shape does not overlap
            if(!overlap(&p1, &p2)){
                doesShapeOverlap = false;
                break;
            }
        }
        if(!doesShapeOverlap) continue;

        for(int j = 0; j < 3; j++) {
            b2Vec2 p1 = projectShape(t, &axes2[j]);
            printf("rprojX: %f, rprojY: %f\n", p1.x, p1.y);
            b2Vec2 p2 = projectShape(colliderShapes[i], &axes2[j]);
            printf("cprojX: %f, cprojY: %f\n", p2.x, p2.y);

            //If projection does not overlap then shape does not overlap
            if(!overlap(&p1, &p2)){
                doesShapeOverlap = false;
                break;
            }
        }
        free(axes2);
        if(doesShapeOverlap) {
            free(colliderShapes);
            free(axes1);
            return true;
        }
    }
    free(colliderShapes);
    free(axes1);
    return false;
}

bool isOverlapping1D(float min1, float max1, float min2, float max2){
    return max1 >=min2 && max2 >= min1;
}

bool isOverlapping2D(SDL_FRect* t, Collider* c){
    int shapeCount = b2Body_GetShapeCount(c->colliderId);
    b2Vec2 colliderPosition = b2Body_GetPosition(c->colliderId);
    b2ShapeId* colliderShapes = (b2ShapeId*)malloc(shapeCount);
    b2Body_GetShapes(c->colliderId, colliderShapes, shapeCount);
    std::vector<b2Vec2> vertices = std::vector<b2Vec2>();

    for(int i = 0; i < shapeCount; i++) {      
		b2Vec2* colliderVertices = b2Shape_GetPolygon(colliderShapes[i]).vertices;
        if(i == shapeCount-1) {
            vertices.push_back(colliderVertices[0]);
        }
        vertices.push_back(colliderVertices[1]);
        if(i == 0) {
            vertices.push_back(colliderVertices[2]);
        }
    }
    free(colliderShapes);

    float minX = t->x;
    float maxX = t->x + t->w;
    float minY = t->y;
    float maxY = t->y + t->h;

    for (const auto& vert : vertices) {
        if (vert.x >= minX && vert.x <= maxX &&
            vert.y >= minY && vert.y <= maxY) {
            return true;
        }
    }
    return false;
}
#pragma endregion