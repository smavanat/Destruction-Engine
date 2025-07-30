#include "GridManager.h"
GridSystemManager::GridSystemManager() {
    gSystem = nullptr;
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
    //grid->tileHeight = tHeight;
    grid->gridWidth = gWidth;
    grid->gridHeight = gHeight;
    grid->tiles = std::vector<TileData>( static_cast<size_t>(gWidth * gHeight) );

    //Making sure the child systems all have the same pointer
    gSystem->setGrid(grid);
    pSystem->setGrid(grid);
}

GridSystemManager::GridSystemManager(int tWidth, int gWidth, int gHeight, std::string path) : GridSystemManager(tWidth, gWidth, gHeight) {
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

//Essentially, this goes over every subcell in a grid tile, and then checks if a given tile overlaps with that specific 
//grid cell, (and how much? -> Could use Greiner–Hormann algorithm to essentially get the intersection area, calc area of 
//the polygon, and then use that to determine whether the tile is filled or not)
#pragma region SubOverlaps
//This is going to be the spot for my implementation of the Griener-Hormann clipping alogorithm. Unfortunately necessary:
bool pointInPolygon(b2Vec2* p, GHVertex* head) {
    bool inside = false;
    GHVertex* v = head;
    do {
        b2Vec2 a = v->p;
        b2Vec2 b = v->next->p;
        if(((a.y > v->p.y) != (b.y > v->p.y)) && (v->p.x < (b.x - a.x) * (v->p.y - a.y) / (b.y - a.y)+a.x)) 
            inside = !inside;
        v = v->next;
    } while (v != head);
    return inside;
}

b2Vec2 intersectSeg(b2Vec2* A, b2Vec2* B, b2Vec2* C, b2Vec2* D, double* t) {
    // compute intersection AB∩CD, return at A + t*(B−A)
    double a1 = B->y - A->y;
    double b1 = A->x - B->x;
    double c1 = a1 * A->x + b1 * A->y;
    double a2 = D->y - C->y;
    double b2 = C->x - D->x;
    double c2 = a2 * C->x + b2 * C->y;
    double det = a1 * b2 - a2 * b1;
    assert(std::fabs(det) > 1e-9);
    double ix = (b2 * c1 - b1 * c2) / det;
    double iy = (a1 * c2 - a2 * c1) / det;
    *t = (std::fabs(B->x-A->x) > std::fabs(B->y-A->y)) ? (ix - A->x)/(B->x - A->x) : (iy - A->y)/(B->y - A->y);
    return {ix, iy};
}

GHVertex* makeVertexList(b2Vec2* polygonVertices, int size) {
    if(size < 3) return NULL;

    GHVertex* head = nullptr;
    GHVertex* prev = nullptr;

    for(int i = 0; i < size; i++) {
        GHVertex v = (GHVertex){nullptr, nullptr, nullptr, polygonVertices[i], 0.0f, false, false, false};
        if(!head) head = &v;
        if(prev) {
            prev->next = &v;
            v.prev = prev;
        }
        prev = &v;
    }

    head->prev = prev;
    prev->next = head;
    return head;
}

//No clue if this works. Need to test urgently
std::vector<std::vector<b2Vec2>> grienerHormannClip(b2Vec2* subjVerts, int svSize, b2Vec2* clipVerts, int cvsize) {
    GHVertex* subjHead = makeVertexList(subjVerts, svSize);
    GHVertex* clipHead = makeVertexList(clipVerts, cvsize);

    for(GHVertex* sp = subjHead;; sp = sp->next) {
        GHVertex* sN = sp->next;
        for (GHVertex* cp = clipHead; ; cp = cp->next) {
            GHVertex* cN = cp->next;
            double tS, tC;
            // segment-segment intersection test with proper ranges
            // (omitting full check for brevity)
            b2Vec2 ip = intersectSeg(&sp->p, &sN->p, &cp->p, &cN->p, &tS);
            if (tS > 0 && tS < 1 /* and similarly tC in (0,1) */) {
                // insert intersection into both lists
                GHVertex vS = (GHVertex){nullptr, nullptr, nullptr, ip, 0.0f, true, false, false};
                GHVertex vC = (GHVertex){nullptr, nullptr, nullptr, ip, 0.0f, true, false, false};
                vS.alpha = tS;
                vC.alpha = tC;
                vS.neighbour = &vC;
                vC.neighbour = &vS;
                // insert into s-list at correct position by alpha (omitted)
                // insert into c-list similarly
            }
            if (cp->next == clipHead) break;
        }
        if (sp->next == subjHead) break;
    }

    // Phase 2: mark entry/exit
    bool subjFirstInside = pointInPolygon(&subjHead->p, clipHead);
    bool clipFirstInside = pointInPolygon(&clipHead->p, subjHead);
    // walk sHead's intersections in order and toggle entry flags
    bool flag = !subjFirstInside;
    for (GHVertex* v = subjHead; ; v = v->next) {
        if (v->intersect) {
            v->entry = flag;
            flag = !flag;
        }
        if (v->next == subjHead) break;
    }
    // similarly for clip polygon intersections

    // Phase 3: construct result polygons
    std::vector<std::vector<b2Vec2>> result;
    for (GHVertex* v = subjHead; ; v = v->next) {
        if (v->intersect && !v->visited && v->entry) {
            std::vector<b2Vec2> out;
            GHVertex* curr = v;
            bool forward = true;
            do {
                curr->visited = true;
                out.push_back(curr->p);
                curr = (forward ? curr->next : curr->prev);
                if (!curr->visited && curr->intersect) {
                    forward = curr->entry;
                    curr = curr->neighbour;
                }
            } while (curr != v);
            result.push_back(out);
        }
        if (v->next == subjHead) break;
    }
    return result;
}

//Need to make main "insertion" function that takes a TileData struct and a Collider reference, then loops over every subcell
//to determine whether they are intersecting or not (can just use SAT algorithm)
void intersectingSubcells(std::shared_ptr<GridData> g, int index, Collider* c) {
    //Loop over the subcells
    for(int i = 0; i < g->tileWidth * g->tileWidth; i++) {
        //Get current subcell world position
        Vector2 subPos = gridToWorldPos(g, index);
        int sWidth = g->tileWidth/g->subWidth;
        subPos.x += sWidth*(i % g->subWidth);
        subPos.y += sWidth*(i / g->subWidth);
        //Generate the rect to be used to represent the subcell
        SDL_FRect subRect = (SDL_FRect){subPos.x, subPos.y, sWidth, sWidth};
        //Check if that rect overlaps with the collider
        if(isOverlapping(&subRect, c)) {
            //If it does, check by how much using Greiner-Hormann algorithm: https://www.inf.usi.ch/hormann/papers/Greiner.1998.ECO.pdf


            //If >=20% (arbitrary value, should be adjusted or made adjustable as needed), set the subcell to be unwalkable
        }
    }
}
#pragma endregion

//This region contains the code to determine whether a given colliders overlaps with a tile in a grid
//(represented by a rect) using the Seperating Axis Theorem. Based on this code: https://dyn4j.org/2010/01/sat/
#pragma region SAT
b2Vec2* getSeperatingAxes(b2ShapeId id, b2Vec2* pos) {
    b2Vec2* axes = (b2Vec2*)calloc(3, sizeof(b2Vec2));
    b2Vec2* colliderVertices = b2Shape_GetPolygon(id).vertices;
    for(int i = 0; i < 3; i++) {
        //Get the current vertex
        b2Vec2 start = (b2Vec2){(colliderVertices[i].x+pos->x)*metresToPixels,(colliderVertices[i].y+pos->y)*metresToPixels};
        //Get the next vertex
        b2Vec2 end = (b2Vec2){(colliderVertices[(i + 1) % 3].x+pos->x)*metresToPixels,(colliderVertices[(i + 1) % 3].y+pos->y)*metresToPixels};
        //Subtract the two to get the edge vector
        b2Vec2 edge = (b2Vec2){(end.x - start.x), (end.y - start.y)};
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

b2Vec2 projectShape(b2ShapeId id, b2Vec2* axis, b2Vec2* pos) {
    b2Vec2* colliderVertices = b2Shape_GetPolygon(id).vertices;
    double min = (axis->x * (colliderVertices[0].x + pos->x)*metresToPixels) + (axis->y * (colliderVertices[0].y + pos->y)*metresToPixels);
    double max = min;

    for(int i = 1; i < 3; i++) {
        double c = (axis->x * (colliderVertices[i].x + pos->x)*metresToPixels) + (axis->y * (colliderVertices[i].y + pos->y)*metresToPixels);
        if(c < min) min = c;
        else if(c > max) max = c;
    }
    
    return (b2Vec2){min, max}; 
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

//Checks if two 1D vectors overlap
bool overlap(b2Vec2* a, b2Vec2* b) {
    return a->x <= b->y && b->x <= a->y;
}

//Checks if a Tile(here represented by an SDL_FRect that represents its four vertices)
//and a collider overlap using the separating axes theorem
bool isOverlapping(SDL_FRect* t, Collider* c) {
    //Axes of the rect always the same, can generate them outside the loop
    b2Vec2* axes1 = getSeperatingAxes(t);

    int shapeCount = b2Body_GetShapeCount(c->colliderId);
    b2Vec2 colliderPosition = b2Body_GetPosition(c->colliderId);
    b2ShapeId* colliderShapes = (b2ShapeId*)malloc(shapeCount*sizeof(b2ShapeId));
    b2Body_GetShapes(c->colliderId, colliderShapes, shapeCount);

    //Check for overlaps for each of the triangle shapes that make up the collider.
    //Cannot just check collider vertices, as this would not account for convex colliders
    //that are going to be generated by destruction 
    for(int i = 0; i < shapeCount; i++) {
        //Axes of the current shape we are testing
        b2Vec2* axes2 = getSeperatingAxes(colliderShapes[i], &colliderPosition);
        bool doesShapeOverlap = true; //Overlap flag set to to true by default 

        //Check that shape axes projection overlaps with rect
        for(int j = 0; j < 4; j++) {
            b2Vec2 p1 = projectShape(t, &axes1[j]);
            b2Vec2 p2 = projectShape(colliderShapes[i], &axes1[j], &colliderPosition);
            //If projection does not overlap then shape does not overlap
            if(!overlap(&p1, &p2)){
                doesShapeOverlap = false;
                break;
            }
        }
        if(!doesShapeOverlap) continue; //If shape does not overlap, then we can skip the next check

        //Check that rect axes projection overlaps with shape
        for(int j = 0; j < 3; j++) {
            b2Vec2 p1 = projectShape(t, &axes2[j]);
            b2Vec2 p2 = projectShape(colliderShapes[i], &axes2[j], &colliderPosition);

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
#pragma endregion