#include "PathSystem.h"
#include <algorithm>
//Pathfinding system initialisation. Incomplete
void PathFindingSystem::init() {

}

void PathFindingSystem::update(float dt) {
    //Need to find a better way of doing this. This is just a quick version that works for now. Use a queue maybe?
    //And then have a function that adds PathFinding requests Entities to the queue or something
    for (Entity e : registeredEntities) {
        Pathfinding &p = gCoordinator.getComponent<Pathfinding>(e);

        //If the component does not have a well defined start and end pos, continue
        if ((p.startPos.x == -1 && p.startPos.y == -1) || (p.endPos.x == -1 && p.endPos.y == -1)) {
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
            auto path = FindPath(p.startPos, p.endPos, grid, p.size);
            std::vector<Vector2> vecPath(path.size());
            int cWidth = grid->tileWidth;

            //Convert it to coordinates
            std::transform(path.begin(), path.end(), vecPath.begin(),
                [cWidth](Node n) {return nodeToWorldPos(n, cWidth); });

            for (Vector2 v : vecPath) {
                printf("(%f, %f)", v.x, v.y);
            }

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