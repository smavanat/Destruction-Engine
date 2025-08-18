#include "PathSystem.h"
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <unordered_map>
#include "BasicComponents.h"
#include "Debug.h"
#include "Event.h"
#include "SDL3/SDL_rect.h"
//Pathfinding system initialisation. Incomplete
void PathFindingSystem::init() {

}

void PathFindingSystem::update(float dt) {
    //Need to find a better way of doing this. This is just a quick version that works for now. Use a queue maybe?
    //And then have a function that adds PathFinding requests Entities to the queue or something
    for (Entity e : registeredEntities) {
    std::unordered_map<std::pair<int, int>, Direction8, Vector2Hasher> directionMap = {{std::make_pair(-1, -1), NW}, {std::make_pair(0, -1), N},
                                                                                    {std::make_pair(1, -1), NE},  {std::make_pair(-1, 0), W},
                                                                                    {std::make_pair(1, 0), E},   {std::make_pair(-1, 1), SW}, 
                                                                                    {std::make_pair(0, 1), S},   {std::make_pair(1, 1), SE}};
        Pathfinding &p = gCoordinator.getComponent<Pathfinding>(e);

        //If the component does not have a well defined start and end pos, continue
        if ((p.startPos.x == -1 && p.startPos.y == -1) || (p.endPos.x == -1 && p.endPos.y == -1)) {
            if(p.path.size() > 0) {
                Transform t = gCoordinator.getComponent<Transform>(e);
                Collider c = gCoordinator.getComponent<Collider>(e);

                if(b2Distance((Vector2){t.position.x*pixelsToMetres, t.position.y*pixelsToMetres},
                                (Vector2){p.path.front().x*pixelsToMetres, p.path.front().y*pixelsToMetres}) < 0.5f){
                    p.path.erase(p.path.begin());
                    if(p.path.size() == 0) {
                        b2Body_SetLinearDamping(c.colliderId, 5.0f);
                        continue;
                    }
                }

                //This whole part here is so hacky and poor I hate it. But it is necessary since I neither have the time nor the energy to think of something better for now
                int gridData = worldToGridIndex(grid,(Vector2){p.path.front().x, p.path.front().y});
                if(grid->tiles[gridData].status != 0) {
                    Vector2 cPos = worldToGridPos(grid, t.position);
                    Vector2 nPos = worldToGridPos(grid, p.path.front());
                    if(cPos.x - nPos.x == 0 && cPos.y - nPos.y == 0) continue;
                    auto direction = directionMap.at(std::make_pair(cPos.x - nPos.x, cPos.y - nPos.y));
                    //Destroy somehow at this direction
                    DestructionEvent* dt = (DestructionEvent*)malloc(sizeof(DestructionEvent));
                    SDL_FRect* q = (SDL_FRect*)malloc(sizeof(SDL_FRect));
                    q->x = p.path.front().x - 10;
                    q->y = p.path.front().y - 20;
                    q->h = 200;
                    q->w = 50;
                    dt->quad = q;
                    switch (direction) {
                        case N:
                        case S:
                            dt->rot = 0;
                        break;
                        case E:
                        case W:
                            dt->rot = 90;
                        break;
                        case NW:
                        case SE:
                            dt->rot = -45;
                        break;
                        case NE:
                        case SW:
                            dt->rot = 45;
                        break;
                    }
                    gCoordinator.getEventBus()->publish(dt);
                }

                Vector2 dir = (Vector2){p.path.front().x - t.position.x, p.path.front().y - t.position.y};

                normalise(&dir);

                float speed = 0.3f;

                b2Body_SetLinearVelocity(c.colliderId, dir*(dt*speed));
            }
            else {
                continue;
            }
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
            // auto path = FindPath(p.startPos, p.endPos, grid, p.size);
            auto path = FindPathDestruction(p.startPos, p.endPos, grid, p.size);
            std::vector<Vector2> vecPath(path.size());
            int cWidth = grid->tileWidth;

            //Convert it to coordinates
            std::transform(path.begin(), path.end(), vecPath.begin(),
                [cWidth](Node n) {return nodeToWorldPos(n, cWidth); });

            for (Vector2 v : vecPath) {
                printf("(%f, %f)", v.x, v.y);
            }
            printf("\n");

            //Set the path value in the component
            p.path = vecPath;
            //Reset the start and endpos to default values to avoid repeat computations
            p.startPos.x = -1;
            p.startPos.y = -1;
            p.endPos.x = -1;
            p.endPos.y = -1;
        }
    }
}

void PathFindingSystem::setGrid(std::shared_ptr<GridData> g) {
    grid = g;
}
