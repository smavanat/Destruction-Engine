#include "GridManager.h"
#include "martinez.h"
GridSystemManager::GridSystemManager() {
    //gSystem = nullptr;
    pSystem = nullptr;
    //tSystem = nullptr;
    grid = std::make_shared<GridData>();

    //Initialising the grid height
    grid->subWidth = 0;
    grid->tileWidth = 0;
    //grid->tileHeight = 0;
    grid->gridWidth = 0;
    grid->gridHeight = 0;
    grid->tiles = std::vector<TileData>( static_cast<size_t>(grid->gridWidth * grid->gridHeight) );
}

GridSystemManager::GridSystemManager(int tWidth, int gWidth, int gHeight) {
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
    //grid->tileHeight = tHeight;
    grid->gridWidth = gWidth;
    grid->gridHeight = gHeight;
    grid->tiles = std::vector<TileData>( static_cast<size_t>(gWidth * gHeight) );

    //Making sure the child systems all have the same pointer
    pSystem->setGrid(grid);
}

GridSystemManager::GridSystemManager(int tWidth, int gWidth, int gHeight, std::string path) : GridSystemManager(tWidth, gWidth, gHeight) {
    loadGridFromFile(path);
}

void GridSystemManager::update(float dt) {
    pSystem->update(dt);
}

void GridSystemManager::setGridTileColliders(TerrainSet* tSet) {
    //Can literally just get the tile from the collider's position by using worldToGridIndex()
    //printf("Number of Terrain Tiles: %i\n", tSet->size); //-> 28
    for(int i = 0; i < tSet->size; i++) {
        Entity e = (Entity){tSet->eArr[i]};
        Collider c = gCoordinator.getComponent<Collider>(e);
        Transform t = gCoordinator.getComponent<Transform>(e);
        Vector2 gridPosition = worldToGridPos(grid, t.position);

        SDL_FRect dimensions = {gridPosition.x*grid->tileWidth, gridPosition.y*grid->tileWidth, grid->tileWidth, grid->tileWidth};
        printf("TilePosition: (%f, %f)\n", dimensions.x, dimensions.y);

        if(isOverlapping(&dimensions, &c)){
            intersectingSubcells(grid, worldToGridIndex(grid, (Vector2){dimensions.x, dimensions.y}), &c, true); //Why isn't this working?
        }
    }
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

void GridSystemManager::printWorldGrid() {
    for(int i = 0; i < grid->gridHeight; i++) {//Looping over the grid height
        for(int p = 0; p < grid->subWidth; p++) { //Looping over the subcell width
            for(int k = 0; k < grid->gridWidth; k++) { //Looping over the grid width
                for(int j = 0; j < grid->subWidth; j++) { //Looping over the subcell height
                    printf("%i ", grid->tiles[(i*grid->gridWidth)+k].subcells[(j*grid->subWidth)+p]);
                }
            }
            printf("\n");
        }
    }
}

//Essentially, this goes over every subcell in a grid tile, and then checks if a given tile overlaps with that specific 
//grid cell, (and how much? -> Could use Greiner–Hormann algorithm to essentially get the intersection area, calc area of 
//the polygon, and then use that to determine whether the tile is filled or not)
#pragma region SubOverlaps
//function to get all unique collider vertices:
std::vector<Point> getColliderVertices(Collider* c) {
    //Getting the collider data
    int shapeCount = b2Body_GetShapeCount(c->colliderId);
    Vector2 colliderPosition = b2Body_GetPosition(c->colliderId);
    b2ShapeId* colliderShapes = (b2ShapeId*)malloc(shapeCount*sizeof(b2ShapeId));
    b2Body_GetShapes(c->colliderId, colliderShapes, shapeCount);
    std::vector<Point> ret = std::vector<Point>();

    //Getting all of the unique collider points
    //Need to figure this part out in a smart way
    for(int i = 0; i < shapeCount; i++) {
        Vector2* points = b2Shape_GetPolygon(colliderShapes[i]).vertices;
        for(int j = 0; j < 3; j++) {
            Point curr = Point(points[j].x, points[j].y);
            if(std::find(ret.begin(), ret.end(), curr) == ret.end())
                ret.push_back(curr);
        }
    }
    return ret;
}

//Transforms an SDL_FRect into an array of points that represent its four corners
std::vector<Point> getRectVertices(SDL_FRect* rect) {
    return std::vector<Point>{Point(rect->x, rect->y), Point(rect->x + rect->w, rect->y), Point(rect->x+rect->w, rect->y+rect->h), 
                                Point(rect->x, rect->y + rect->h)};
}

//My implementation of the shoelace formula to get the area of a polygon
float getPolygonArea(Polygon& p) {
    float sum1 = 0.0f;
    float sum2 = 0.0f;

    for(int i = 0; i < p.ncontours(); i++) {
		for(int j = 0; j < p[i].nvertices()-1; j++) {
            sum1 += p[i].vertex(j).x * p[i].vertex(j == p[i].nvertices() ? 0 : j + 1).y;
            sum2 += p[i].vertex(j).y * p[i].vertex(j == p[i].nvertices() ? 0 : j + 1).x;
		}
	}

    return b2AbsFloat(sum1 - sum2) / 2.0f;
}

//This function is not actually updating the tiledata. Why?

//Need to make main "insertion" function that takes a TileData struct and a Collider reference, then loops over every subcell
//to determine whether they are intersecting or not (can just use SAT algorithm)
//Need to rework this so it batches computations to make it work better with SIMD
void intersectingSubcells(std::shared_ptr<GridData> g, int index, Collider* c, bool setUnwalkable) {
    printf("xPos: %i", index % g->gridWidth);
    //Loop over the subcells
    for(int i = 0; i < g->subWidth * g->subWidth; i++) {
        //Get current subcell world position
        Vector2 subPos = gridToWorldPos(g, index);
        int sWidth = g->tileWidth/g->subWidth;
        subPos.x += sWidth*(i % g->subWidth);
        subPos.y += sWidth*(i / g->subWidth);
        //Generate the rect to be used to represent the subcell
        SDL_FRect subRect = (SDL_FRect){subPos.x, subPos.y, sWidth, sWidth};
        printf("Subcell No.%i position: (%f, %f)\n", i, subPos.x, subPos.y);
        //Check if that rect overlaps with the collider
        if(isOverlapping(&subRect, c)) { //This is now sometimes false. Need to check why
            printf("Subcell is overlapping\n");
            //If it does, check by how much using Martinez-Rueda-Fiedo algorithm: https://www.sciencedirect.com/science/article/abs/pii/S0098300408002793
	        std::vector<Point> rectPoints = getRectVertices(&subRect);
            Polygon testRectPoly = Polygon(rectPoints);
            std::vector<Point> colliderPoints = getColliderVertices(c);
            Polygon testColliderPoly = Polygon(colliderPoints);
            Polygon testResult = Polygon();
            Martinez compute = Martinez(testRectPoly, testColliderPoly);
            compute.compute(compute.INTERSECTION, testResult);

            for(int j = 0; j < testResult.ncontours(); j++) {
                for(int k = 0; k < testResult[j].nvertices(); k++) {
                    printf("(%f, %f) ", testResult[j].vertex(k).x, testResult[j].vertex(k).y);
                }
            }
            printf("\n");

            //If we are checking for collider overlaps to set unwalkable/walkable tiles (on world instantiation or collider creation)
            if(setUnwalkable) {
                //If >=20% (arbitrary value, should be adjusted or made adjustable as needed), set the subcell to be unwalkable
                if(getPolygonArea(testResult) / getPolygonArea(testRectPoly) >= 0.2) 
                    g->tiles[index].subcells[i] = 1;
                else 
                    g->tiles[index].subcells[i] = 0;
            }
            //If a collider is destroyed, then we can safely assume that all tiles that it overlaps are walkable (right?)
            else {
                g->tiles[index].subcells[i] = 0;
            }

            //Need to add code here that makes all other subcells unfilled.
        }
   }
}
#pragma endregion

//This region contains the code to determine whether a given colliders overlaps with a tile in a grid
//(represented by a rect) using the Seperating Axis Theorem. Based on this code: https://dyn4j.org/2010/01/sat/
//Why use SAT to get overlap initially when we know both things will be bounding boxes with 0 rotation. Why not just
//use bounding box collisions and use SAT at more complex times?
#pragma region SAT
Vector2* getSeperatingAxes(b2ShapeId id, Vector2* pos) {
    Vector2* axes = (Vector2*)calloc(3, sizeof(Vector2));
    Vector2* colliderVertices = b2Shape_GetPolygon(id).vertices;
    for(int i = 0; i < 3; i++) {
        //Get the current vertex
        Vector2 start = (Vector2){(colliderVertices[i].x+pos->x)*metresToPixels,(colliderVertices[i].y+pos->y)*metresToPixels};
        //Get the next vertex
        Vector2 end = (Vector2){(colliderVertices[(i + 1) % 3].x+pos->x)*metresToPixels,(colliderVertices[(i + 1) % 3].y+pos->y)*metresToPixels};
        //Subtract the two to get the edge vector
        Vector2 edge = (Vector2){(end.x - start.x), (end.y - start.y)};
        //Get the normal of the edge
        Vector2 normal = (Vector2){-edge.y, edge.x};
        float length = sqrt(normal.x * normal.x + normal.y * normal.y);
        if (length > 0) {
            normal.x /= length;
            normal.y /= length;
        }
        axes[i] = normal;
    }
    return axes;
}

Vector2* getSeperatingAxes(SDL_FRect* rect) {
    Vector2* axes = (Vector2*)calloc(4, sizeof(Vector2));
    Vector2 colliderVertices[] = {(Vector2){rect->x, rect->y}, (Vector2){rect->x+rect->w, rect->y}, (Vector2){rect->x+rect->w, rect->y+rect->h}, (Vector2){rect->x, rect->y+rect->h}};

    for(int i = 0; i < 4; i++) {
        //Get the current vertex
        Vector2 start = colliderVertices[i];
        //Get the next vertex
        Vector2 end = colliderVertices[(i + 1) % 4];
        //Subtract the two to get the edge vector
        Vector2 edge = (Vector2){end.x - start.x, end.y - start.y};
        //Get the normal of the edge
        Vector2 normal = (Vector2){-edge.y, edge.x};
        float length = sqrt(normal.x * normal.x + normal.y * normal.y);
        if (length > 0) {
            normal.x /= length;
            normal.y /= length;
        }
        axes[i] = normal;
    }
    return axes;
}

Vector2 projectShape(b2ShapeId id, Vector2* axis, Vector2* pos) {
    Vector2* colliderVertices = b2Shape_GetPolygon(id).vertices;
    double min = (axis->x * (colliderVertices[0].x + pos->x)*metresToPixels) + (axis->y * (colliderVertices[0].y + pos->y)*metresToPixels);
    double max = min;

    for(int i = 1; i < 3; i++) {
        double c = (axis->x * (colliderVertices[i].x + pos->x)*metresToPixels) + (axis->y * (colliderVertices[i].y + pos->y)*metresToPixels);
        if(c < min) min = c;
        else if(c > max) max = c;
    }
    
    return (Vector2){min, max}; 
}

Vector2 projectShape(SDL_FRect* rect, Vector2* axis) {
    Vector2 colliderVertices[] = {(Vector2){rect->x, rect->y}, (Vector2){rect->x+rect->w, rect->y}, (Vector2){rect->x+rect->w, rect->y+rect->h}, (Vector2){rect->x, rect->y+rect->h}};
    double min = (axis->x * colliderVertices[0].x) + (axis->y * colliderVertices[0].y);
    double max = min;

    for(int i = 1; i < 4; i++) {
        double c = (axis->x * colliderVertices[i].x) + (axis->y * colliderVertices[i].y);
        if(c < min) min = c;
        else if(c > max) max = c;
    }
    
    return (Vector2){min, max};
}

//Checks if two 1D vectors overlap
bool overlap(Vector2* a, Vector2* b) {
    return a->x <= b->y && b->x <= a->y;
}

//Checks if a Tile(here represented by an SDL_FRect that represents its four vertices)
//and a collider overlap using the separating axes theorem
//Need to rework this to work with all types of colliders
bool isOverlapping(SDL_FRect* t, Collider* c) {
    //Add bounding box checks before doing SAT
    //Axes of the rect always the same, can generate them outside the loop
    Vector2* axes1 = getSeperatingAxes(t);

    int shapeCount = b2Body_GetShapeCount(c->colliderId);
    Vector2 colliderPosition = b2Body_GetPosition(c->colliderId);
    b2ShapeId* colliderShapes = (b2ShapeId*)malloc(shapeCount*sizeof(b2ShapeId));
    b2Body_GetShapes(c->colliderId, colliderShapes, shapeCount);

    //Check for overlaps for each of the triangle shapes that make up the collider.
    //Cannot just check collider vertices, as this would not account for convex colliders
    //that are going to be generated by destruction 
    for(int i = 0; i < shapeCount; i++) {
        //Axes of the current shape we are testing
        Vector2* axes2 = getSeperatingAxes(colliderShapes[i], &colliderPosition);
        bool doesShapeOverlap = true; //Overlap flag set to to true by default 

        //Check that shape axes projection overlaps with rect
        for(int j = 0; j < 4; j++) {
            Vector2 p1 = projectShape(t, &axes1[j]);
            Vector2 p2 = projectShape(colliderShapes[i], &axes1[j], &colliderPosition);
            //If projection does not overlap then shape does not overlap
            if(!overlap(&p1, &p2)){
                doesShapeOverlap = false;
                break;
            }
        }
        if(!doesShapeOverlap) continue; //If shape does not overlap, then we can skip the next check

        //Check that rect axes projection overlaps with shape
        for(int j = 0; j < 3; j++) {
            Vector2 p1 = projectShape(t, &axes2[j]);
            Vector2 p2 = projectShape(colliderShapes[i], &axes2[j], &colliderPosition);

            //If projection does not overlap then shape does not overlap
            if(!overlap(&p1, &p2)){
                doesShapeOverlap = false;
                break;
            }
        }
        //Need to use arenas to make memory management better
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
#pragma endregion
