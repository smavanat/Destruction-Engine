#include "GridManager.h"
#include "Maths.h"
#include "martinez.h"
#include <cstdlib>
#include <memory>
void CreateGridData(std::shared_ptr<GridData> grid, int tWidth, int gWidth, int gHeight) {
    //Initialising the grid height
    grid->subWidth = 8;
    grid->tileWidth = tWidth;
    //grid->tileHeight = tHeight;
    grid->gridWidth = gWidth;
    grid->gridHeight = gHeight;
    grid->tiles = std::vector<TileData>( static_cast<size_t>(gWidth * gHeight) );
}

bool LoadGridFromFile(std::shared_ptr<GridData> grid, std::string path) {
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

void PrintGrid(std::shared_ptr<GridData> grid) {
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

double roundTo3dp(double value) {
    return std::round(value * 1000.0) / 1000.0;
}

#pragma region ColliderOrdering
Edge makeEdge(Vector2 a, Vector2 b) {
    if (a.x < b.x || (a.x == b.x && a.y < b.y)) {
        return (Edge){a, b};
    }
    return (Edge){b,a};
}

bool equals(Edge a, Edge b) {
    return equals(a.a, b.a) && equals(a.b, b.b);
}

CountMap* initialiseCountMap() {
    CountMap* cm = (CountMap*)malloc(sizeof(CountMap));

    cm->size = 0;
    cm->maxSize = 10;
    cm->points = (Edge*)calloc(10, sizeof(Edge));
    cm->count = (uint32_t*)calloc(10, sizeof(uint32_t));
    return cm;
}

PointMap* initialisePointMap() {
    PointMap* pm = (PointMap*)malloc(sizeof(PointMap));

    pm->size = 0;
    pm->maxSize = 10;
    pm->forward = (Vector2*)malloc(10*sizeof(Vector2));
    pm->back = (Vector2*)malloc(10*sizeof(Vector2));
    return pm;
}

bool contains(PointMap* pm, Vector2* a, Vector2* b) {
    for(int i =0; i < pm->size; i++) {
        if(equals(pm->forward[i], *a) && equals(pm->back[i], *b))
            return true;
    }
    return false;
}

void insert(PointMap* pm, Vector2* a, Vector2* b) {
    if(contains(pm, a, b)) return;

    if(pm->size >= pm->maxSize) {
        Vector2* tempF = (Vector2*)malloc(pm->maxSize*2*sizeof(Vector2));
        memcpy(tempF, pm->forward, pm->maxSize*sizeof(Vector2));
        free(pm->forward);
        pm->forward = tempF;

        Vector2* tempP = (Vector2*)malloc(pm->maxSize*2*sizeof(Vector2));
        memcpy(tempP, pm->back, pm->maxSize*sizeof(Vector2));
        free(pm->back);
        pm->back = tempP;

        pm->maxSize*=2;
    }
    pm->forward[pm->size] = *a;
    pm->back[pm->size] = *b;
    pm->size++;
}

void add(CountMap* cm, Edge v) {
    for(int i = 0; i < cm->size; i++) {
        if(equals(cm->points[i], v)){
            cm->count[i]++;
            return;
        }
    }
    if(cm->size >= cm->maxSize) {
        uint32_t* tempC = (uint32_t*)calloc(cm->maxSize*2, sizeof(uint32_t));
		memcpy(tempC, cm->count, cm->maxSize*sizeof(uint32_t));
        free(cm->count);
		cm->count = tempC;

        Edge* tempP = (Edge*)malloc(cm->maxSize*2*sizeof(Edge));
		memcpy(tempP, cm->points, cm->maxSize*sizeof(Edge));
        free(cm->points);
		cm->points = tempP;
		cm->maxSize*=2;
    }

    cm->points[cm->size] = v;
    cm->count[cm->size] = 1;
    cm->size++;
}

VectorList* initialiseVectorList() {
    VectorList* vl = (VectorList*)malloc(sizeof(VectorList));

    vl->size = 0;
    vl->maxSize = 10;
    vl->entries = (Vector2*)malloc(10*sizeof(Vector2));
    return vl;
}

void add(VectorList* vl, Vector2* v) {
    if(vl->size >= vl->maxSize) {
        Vector2* temp = (Vector2*)malloc(vl->maxSize*2*sizeof(Vector2));
		memcpy(temp, vl->entries, vl->maxSize*sizeof(Vector2));
        free(vl->entries);
		vl->entries = temp;
		vl->maxSize*=2;
    }

    vl->entries[vl->size] = *v;
    vl->size++;
}

VectorList* getElementsWithEqualKey(PointMap* pm, Vector2 key) {
    VectorList* ret = initialiseVectorList();
    for(int i = 0; i < pm->size; i++) {
        if(equals(pm->forward[i], key)){
            add(ret, &pm->back[i]);
        }
    }
    return ret;
}

VectorList* reconstructPolygonBoundary(Collider* c) {
    //Usual steps of getting collider data
    int shapeCount = b2Body_GetShapeCount(c->colliderId);
    Vector2 colliderPosition = b2Body_GetPosition(c->colliderId);
    b2ShapeId* colliderShapes = (b2ShapeId*)malloc(shapeCount*sizeof(b2ShapeId));
    b2Body_GetShapes(c->colliderId, colliderShapes, shapeCount);

    //Initiallising the countMap for edges
    CountMap* cMap = initialiseCountMap();

    //Adding the edges of each triangle to the countMap
    for(int i = 0; i < shapeCount; i++) {
        Vector2* points = b2Shape_GetPolygon(colliderShapes[i]).vertices;
        Vector2 x = points[0];
        Vector2 y = points[1];
        Vector2 z = points[2];

        add(cMap, makeEdge(x, y));
        add(cMap, makeEdge(x, z));
        add(cMap, makeEdge(y, z));
    }

    PointMap* pMap = initialisePointMap();
    //Extracting the unique Edges from the countMap
    for(int i = 0; i < cMap->size; i++) {
        if(cMap->count[i] == 1) {
            insert(pMap, &cMap->points[i].a, &cMap->points[i].b);
            insert(pMap, &cMap->points[i].b, &cMap->points[i].a);
        }
    }

    VectorList* boundary = initialiseVectorList();
    if(pMap->size == 0) {
        free(pMap->back);
        free(pMap->forward);
        free(pMap);
        free(cMap->points);
        free(cMap);
        free(colliderShapes);
        return boundary;
    }

    Vector2 start = pMap->forward[0];
    Vector2 current = start;
    Vector2 previous = {9999, 9999};

    do {
        add(boundary, &current);

        Vector2 next;
        VectorList* range = getElementsWithEqualKey(pMap, current);
        for(int i = 0; i < range->size; i++) {
            if(!equals(range->entries[i], previous)) {
                next = range->entries[i];
                break;
            }
        }
        previous = current;
        current = next;
        free(range->entries);
        free(range);
    }while(!equals(current, start));

    free(pMap->back);
    free(pMap->forward);
    free(pMap);
    free(cMap->points);
    free(cMap);
    free(colliderShapes);
    return boundary;
}
#pragma endregion

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

    switch(c->type) {
        case BOX: {
            Vector2* points = b2Shape_GetPolygon(colliderShapes[0]).vertices;

            for(int i = 0; i < 4; i++) {
                Point curr = Point(roundTo3dp((points[i].x + colliderPosition.x)*metresToPixels), roundTo3dp((points[i].y+colliderPosition.y)*metresToPixels));
                ret.push_back(curr);
            }
            break;
        }
        case POLYGON: {
            VectorList* polyPoints = reconstructPolygonBoundary(c);
            for(int i = 0; i < polyPoints->size; i++) {
                Point curr = Point(roundTo3dp((polyPoints->entries[i].x + colliderPosition.x)*metresToPixels), roundTo3dp((polyPoints->entries[i].y+colliderPosition.y)*metresToPixels));
                ret.push_back(curr);
            }
            break;
        }
        default:
            break;
    }
    free(colliderShapes);
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
		for(int j = 0; j < p[i].nvertices(); j++) {
            sum1 += p[i].vertex(j).x * p[i].vertex((j + 1) % p[i].nvertices()).y;
            sum2 += p[i].vertex(j).y * p[i].vertex((j + 1) % p[i].nvertices()).x;
		}
	}

    return b2AbsFloat(sum1 - sum2) / 2.0f;
}

//Need to make main "insertion" function that takes a TileData struct and a Collider reference, then loops over every subcell
//to determine whether they are intersecting or not (can just use SAT algorithm)
//Need to rework this so it batches computations to make it work better with SIMD
void intersectingSubcells(std::shared_ptr<GridData> g, Collider* c, bool setUnwalkable, Vector2 start) {
    // printf("Start: (%f, %f)\n", start.x, start.y);
    // printf("Collider Vertices:\n");
    // int numVertices = 0;
    // switch(c->type) {
    //     case POLYGON:
    //         numVertices = 3;
    //         break;
    //     case BOX:
    //         numVertices = 4;
    //         break;
    //     default:
    //         break;
    // }
    // int shapeCount = b2Body_GetShapeCount(c->colliderId);
    // Vector2 colliderPosition = b2Body_GetPosition(c->colliderId);
    // b2ShapeId* colliderShapes = (b2ShapeId*)malloc(shapeCount*sizeof(b2ShapeId));
    // b2Body_GetShapes(c->colliderId, colliderShapes, shapeCount);
    // for(int j = 0; j < shapeCount; j++) {
    //     Vector2* verts = b2Shape_GetPolygon(colliderShapes[j]).vertices;
    //     for(int i = 0; i < numVertices; i++) {
    //         printf("(%f, %f) ", (verts[i].x+colliderPosition.x)*metresToPixels, (verts[i].y + colliderPosition.y)*metresToPixels);
    //     }
    // }
    // printf("\n");

    std::vector<Point> colliderPoints = getColliderVertices(c);
    Polygon testColliderPoly = Polygon(colliderPoints);

    int index = worldToGridIndex(g, start);
    //Loop over the subcells
    for(int i = 0; i < g->subWidth * g->subWidth; i++) {
        //Get current subcell world position
        Vector2 subPos = start;
        int sWidth = g->tileWidth/g->subWidth;
        subPos.x += sWidth*(i % g->subWidth);
        subPos.y += sWidth*(i / g->subWidth);
        //Generate the rect to be used to represent the subcell
        SDL_FRect subRect = (SDL_FRect){subPos.x, subPos.y, sWidth, sWidth};
        //printf("Subcell No.%i position: (%f, %f) (%f, %f)\n", i, subRect.x, subRect.y, subRect.w, subRect.h);

        //Check if that rect overlaps with the collider
        if(isOverlapping(&subRect, c)) { 
            //printf("Subcell is overlapping\n");
            //If it does, check by how much using Martinez-Rueda-Fiedo algorithm: https://www.sciencedirect.com/science/article/abs/pii/S0098300408002793
	        std::vector<Point> rectPoints = getRectVertices(&subRect);
            Polygon testRectPoly = Polygon(rectPoints);
            Polygon testResult = Polygon();
            Martinez compute = Martinez(testRectPoly, testColliderPoly);
            compute.compute(compute.INTERSECTION, testResult); 

            // printf("Rect Polygon Vertices:\n");
            // for(int i = 0; i < testRectPoly.ncontours(); i++) {
            //     for(int j = 0; j < testRectPoly[i].nvertices(); j++) {
            //         printf("(%f, %f)", testRectPoly[i].vertex(j).x, testRectPoly[i].vertex(j).y);
            //     }
            // }
            // printf("\nCollider Polygon Vertices: \n");
            // for(int i = 0; i < testColliderPoly.ncontours(); i++) {
            //     for(int j = 0; j < testColliderPoly[i].nvertices(); j++) {
            //         printf("(%f, %f)", testColliderPoly[i].vertex(j).x, testColliderPoly[i].vertex(j).y);
            //     }
            // }
            // printf("\nResult Polygon Vertices: \n");
            // for(int i = 0; i < testResult.ncontours(); i++) {
            //     for(int j = 0; j < testResult[i].nvertices(); j++) {
            //         printf("(%f, %f)", testResult[i].vertex(j).x, testResult[i].vertex(j).y);
            //     }
            // }
            // printf("\n");

            //If we are checking for collider overlaps to set unwalkable/walkable tiles (on world instantiation or collider creation)
            //This doesn't quite work when tiles are being destroyed. I think its because of the faulty positioning
            if(setUnwalkable) {
                float resArea = getPolygonArea(testResult);
                float rectArea = getPolygonArea(testRectPoly);
                // printf("Area of the resulting polygon: %f\n", resArea);
                // printf("Area of the tile rect: %f\n", rectArea);
                //If >=40% (arbitrary value, should be adjusted or made adjustable as needed), set the subcell to be unwalkable
                if(resArea / rectArea >= 0.4)
                    g->tiles[index].subcells[i] = 1;
                else
                    g->tiles[index].subcells[i] = 0;
            }
            //If a collider is destroyed, then we can safely assume that all tiles that it overlaps are walkable (right?)
            else {
                g->tiles[index].subcells[i] = 0;
            }

        }
        //Need to add code that re-evaluates the status of the tile (walkable, unwalkable or partial)
        //Need to add code that re-evaluates the exitable statuses of the tile
        if(setUnwalkable) {
            //Resetting exitable status of the tile's edges:
            g->tiles[index].exitable = {false, false, false, false};
            //Setting the status of the tile
            int walkable = 0;
            int unwalkable = 0;
            for(int i = 0; i < g->subWidth * g->subWidth; i++) {
                if(g->tiles[index].subcells[i] == 0) {
                    walkable++;
                    //Setting the exitable statuses of the tile's edges
                    if(i / g->subWidth == 0) g->tiles[index].exitable[0] = true; //North
                    if(i % g->subWidth == g->subWidth-1) g->tiles[index].exitable[1] = true; //East
                    if(i / g->subWidth == g->subWidth-1) g->tiles[index].exitable[2] = true; //South
                    if(i % g->subWidth == 0) g->tiles[index].exitable[3] = true; //West
                }
                else unwalkable++;
            }
            if(walkable == g->subWidth*g->subWidth) g->tiles[index].status = 0; //set the tile to be walkable
            else if (unwalkable == g->subWidth*g->subWidth) g->tiles[index].status = 1; //set the tile to be unwalkable
            else g->tiles[index].status = 2; //set the tile to be partial
        }
    }
    // for(int i = 0; i < g->subWidth; i++) {
    //     for(int j = 0; j < g->subWidth; j ++) {
    //         printf("%i ", g->tiles[index].subcells[(i*g->subWidth)+j]);
    //     }
    //     printf("\n");
    // }
}
#pragma endregion

//This region contains the code to determine whether a given colliders overlaps with a tile in a grid
//(represented by a rect) using the Seperating Axis Theorem. Based on this code: https://dyn4j.org/2010/01/sat/
//Why use SAT to get overlap initially when we know both things will be bounding boxes with 0 rotation. Why not just
//use bounding box collisions and use SAT at more complex times?
#pragma region SAT
Vector2* convertRectToQuad(SDL_FRect* t, float r) {
    Vector2* ret = (Vector2*)malloc(sizeof(Vector2));
    Vector2 colliderVertices[] = {(Vector2){t->x, t->y}, (Vector2){t->x+t->w, t->y}, (Vector2){t->x+t->w, t->y+t->h}, (Vector2){t->x, t->y+t->h}};
    Vector2 rCentre = {t->x + (t->w/2.0f), t->y + (t->h/2.0f)};
    
    for(int i = 0; i < 4; i ++) {
        ret[0] = rotateAboutPoint(&colliderVertices[i], &rCentre, r, false);
    }

    return ret;
}

Vector2* getSeperatingAxes(b2ShapeId id, Vector2* pos, int numVertices) {
    Vector2* axes = (Vector2*)calloc(numVertices, sizeof(Vector2));
    Vector2* colliderVertices = b2Shape_GetPolygon(id).vertices;
    for(int i = 0; i < numVertices; i++) {
        //Get the current vertex
        Vector2 start = (Vector2){(colliderVertices[i].x+pos->x)*metresToPixels,(colliderVertices[i].y+pos->y)*metresToPixels};
        //Get the next vertex
        Vector2 end = (Vector2){(colliderVertices[(i + 1) % numVertices].x+pos->x)*metresToPixels,(colliderVertices[(i + 1) % numVertices].y+pos->y)*metresToPixels};
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

Vector2* getSeperatingAxes(Vector2* rect) {
    Vector2* axes = (Vector2*)calloc(4, sizeof(Vector2));

    for(int i = 0; i < 4; i++) {
        //Get the current vertex
        Vector2 start = rect[i];
        //Get the next vertex
        Vector2 end = rect[(i + 1) % 4];
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
Vector2 projectShape(b2ShapeId id, Vector2* axis, Vector2* pos, int numVertices) {
    Vector2* colliderVertices = b2Shape_GetPolygon(id).vertices;
    double min = (axis->x * (colliderVertices[0].x + pos->x)*metresToPixels) + (axis->y * (colliderVertices[0].y + pos->y)*metresToPixels);
    double max = min;

    for(int i = 1; i < numVertices; i++) {
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

Vector2 projectShape(Vector2* rect, Vector2* axis) {
    double min = (axis->x * rect[0].x) + (axis->y * rect[0].y);
    double max = min;

    for(int i = 1; i < 4; i++) {
        double c = (axis->x * rect[i].x) + (axis->y * rect[i].y);
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

    int numVertices = 0;
    switch(c->type) {
        case POLYGON: 
            numVertices = 3; //Since all polygons are made up of triangles
            break;
        case BOX: 
            numVertices = 4;
            break;
        default:
            break;
    }

    //Getting collider data
    //Never account for tile rotation. Might be useful in future
    int shapeCount = b2Body_GetShapeCount(c->colliderId);
    Vector2 colliderPosition = b2Body_GetPosition(c->colliderId);
    b2ShapeId* colliderShapes = (b2ShapeId*)malloc(shapeCount*sizeof(b2ShapeId));
    b2Body_GetShapes(c->colliderId, colliderShapes, shapeCount);

    //Check for overlaps for each of the triangle shapes that make up the collider.
    //Cannot just check collider vertices, as this would not account for convex colliders
    //that are going to be generated by destruction
    for(int i = 0; i < shapeCount; i++) {
        //Axes of the current shape we are testing
        Vector2* axes2 = getSeperatingAxes(colliderShapes[i], &colliderPosition, numVertices);
        bool doesShapeOverlap = true; //Overlap flag set to to true by default

        //Check that shape axes projection overlaps with rect
        for(int j = 0; j < 4; j++) {
            Vector2 p1 = projectShape(t, &axes1[j]);
            Vector2 p2 = projectShape(colliderShapes[i], &axes1[j], &colliderPosition, numVertices);
            //If projection does not overlap then shape does not overlap
            if(!overlap(&p1, &p2)){
                doesShapeOverlap = false;
                break;
            }
        }
        if(!doesShapeOverlap) continue; //If shape does not overlap, then we can skip the next check

        //Check that rect axes projection overlaps with shape
        for(int j = 0; j < numVertices; j++) {
            Vector2 p1 = projectShape(t, &axes2[j]);
            Vector2 p2 = projectShape(colliderShapes[i], &axes2[j], &colliderPosition, numVertices);

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

bool isOverlapping(SDL_FRect* t1, float r1, SDL_FRect* t2, float r2) {
    //Add bounding box checks before doing SAT
    //Axes of the rect always the same, can generate them outside the loop
    Vector2* q1 = convertRectToQuad(t1, r1);
    Vector2* q2 = convertRectToQuad(t2, r2);
    Vector2* axes1 = getSeperatingAxes(q1);
    Vector2* axes2 = getSeperatingAxes(q2);

    for(int j = 0; j < 4; j++) {
        Vector2 p1 = projectShape(q1, &axes1[j]);
        Vector2 p2 = projectShape(q2, &axes1[j]);
        //If projection does not overlap then shape does not overlap
        if(overlap(&p1, &p2)){
            free(q1);
            free(q2);
            free(axes1);
            free(axes2);
            return true;
        }
    }

    //Check that rect axes projection overlaps with shape
    for(int j = 0; j < 4; j++) {
        Vector2 p1 = projectShape(q1, &axes2[j]);
        Vector2 p2 = projectShape(q2, &axes2[j]);

        //If projection does not overlap then shape does not overlap
        if(overlap(&p1, &p2)){
            free(q1);
            free(q2);
            free(axes1);
            free(axes2);
            return true;
        }
    }
    free(q1);
    free(q2);
    free(axes1);
    free(axes2);
    return false;
}
#pragma endregion
