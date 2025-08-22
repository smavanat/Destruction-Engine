#include "Debug.h"
#include "SDL3/SDL_render.h"
#include "utils.h"
#pragma region DebugManager

DebugManager::DebugManager() {
	//Initialise all of the Debug Systems
	//These have to be pointers, as otherwise we would be creating two seperate sets of systems, one here 
	//and another in the Coordinator class
	std::shared_ptr<ColliderDebugSystem> cPtr;
	std::shared_ptr<GridDebugSystem> gPtr;
	std::shared_ptr<PathFindingDebugSystem> pPtr;

	//Create the signatures for each of the systems and then create the system using it
	{
		Signature sig;
		sig.addComponent<Collider>();
		cPtr = gCoordinator.addSystem<ColliderDebugSystem>(sig);
	}
	{
		Signature sig;
		sig.addComponent<TileRect>();
		gPtr = gCoordinator.addSystem<GridDebugSystem>(sig);
	}
	{
		Signature sig;
		sig.addComponent<Pathfinding>();
		pPtr = gCoordinator.addSystem<PathFindingDebugSystem>(sig);
	}

	//Assign the pointers to this manager
	registerDebugSystem<ColliderDebugSystem>(cPtr);
	registerDebugSystem<GridDebugSystem>(gPtr);
	registerDebugSystem<PathFindingDebugSystem>(pPtr);
}

//Initialises all the systems stored in the manager
void DebugManager::init() {
	//Maybe there will be something to put in here at some point
}

//The main reason this class was created, to batch run all of these debug systems in one call
void DebugManager::update(float dt) {
	for (auto const& pair : debugMap) {
		if (pair.second) {
			pair.second->update(dt);
		}
	}
}
#pragma endregion

#pragma region ColliderDebugSystem
void ColliderDebugSystem::init() {
	gCoordinator.getEventBus()->subscribe(this, &ColliderDebugSystem::onColliderDebugEvent);
}

void ColliderDebugSystem::onColliderDebugEvent(const ColliderDebugEvent* event) {
	displayColliderOutlines = !displayColliderOutlines;
}

void ColliderDebugSystem::update(float dt) {
	if (displayColliderOutlines) {
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);//Setting the line colour to red
		//For drawing "normal" colliders. Assuming that all colliders are made up of triangles.
		//Triangles I think are the way forward, need to make it so all colliders are made up of triangle 
		//polygons. It just makes things easier.
		for (Entity entity : registeredEntities) {
			Collider c = gCoordinator.getComponent<Collider>(entity);
			//This is the unfortunately very convoluted process of getting the shapes that make up a box2D body
			int shapeCount = b2Body_GetShapeCount(c.colliderId);
			Vector2 colliderPosition = b2Body_GetPosition(c.colliderId);
			b2ShapeId* colliderShapes = new b2ShapeId[shapeCount];
			b2Body_GetShapes(c.colliderId, colliderShapes, shapeCount);

			//Need to draw the different collider types differently
			switch(c.type) {
				case BOX:
					for (int j = 0; j < shapeCount; j++) {
						Vector2* colliderVertices = b2Shape_GetPolygon(colliderShapes[j]).vertices;
						Vector2* rotatedVertices = (Vector2*)malloc(4*sizeof(Vector2));
						for (int k = 0; k < 4; k++) {
							Vector2 temp = rotateTranslate(colliderVertices[k], b2Rot_GetAngle(b2Body_GetRotation(c.colliderId)));
							rotatedVertices[k] = temp;
						}
						for (int k = 0; k < 4; k++) {
							SDL_RenderLine(gRenderer, ((rotatedVertices[k].x + colliderPosition.x) * metresToPixels), ((rotatedVertices[k].y + colliderPosition.y) * metresToPixels),
								((rotatedVertices[(k + 1) > 3 ? 0 : (k + 1)].x + colliderPosition.x) * metresToPixels), ((rotatedVertices[(k + 1) > 3 ? 0 : (k + 1)].y + colliderPosition.y) * metresToPixels));
						}
						free(rotatedVertices);
					}
					break;
				case CIRCLE:
					for(int j = 0; j < shapeCount; j++) {
						b2Circle circle = b2Shape_GetCircle(colliderShapes[j]);
						drawCircle(gRenderer, {(circle.center.x+colliderPosition.x)*metresToPixels, (circle.center.y+colliderPosition.y) * metresToPixels}, circle.radius*metresToPixels);
					}
					break;
				case CAPSULE:
					for(int j = 0; j < shapeCount; j++) {
						b2Capsule capsule = b2Shape_GetCapsule(colliderShapes[j]);
						
						drawCircle(gRenderer, {(capsule.center1.x+colliderPosition.x)*metresToPixels, (capsule.center1.y+colliderPosition.y) * metresToPixels}, capsule.radius*metresToPixels);
						drawCircle(gRenderer, {(capsule.center2.x+colliderPosition.x)*metresToPixels, (capsule.center2.y+colliderPosition.y) * metresToPixels}, capsule.radius*metresToPixels);
					}
					break;
				case POLYGON:
					//Iterate over all of the subshapes, then just draw lines between the vertices
					for (int j = 0; j < shapeCount; j++) {
						Vector2* colliderVertices = b2Shape_GetPolygon(colliderShapes[j]).vertices;
						Vector2* rotatedVertices = (Vector2*)malloc(3*sizeof(Vector2));
						for (int k = 0; k < 3; k++) {
							Vector2 temp = rotateTranslate(colliderVertices[k], b2Rot_GetAngle(b2Body_GetRotation(c.colliderId)));
							rotatedVertices[k] = temp;
						}
						for (int k = 0; k < 3; k++) {
							SDL_RenderLine(gRenderer, ((rotatedVertices[k].x + colliderPosition.x) * metresToPixels), ((rotatedVertices[k].y + colliderPosition.y) * metresToPixels),
								((rotatedVertices[(k + 1) > 2 ? 0 : (k + 1)].x + colliderPosition.x) * metresToPixels), ((rotatedVertices[(k + 1) > 2 ? 0 : (k + 1)].y + colliderPosition.y) * metresToPixels));
						}
						free(rotatedVertices);
					}
					break;
				default:
					break;
			}
			delete[] colliderShapes;
		}
	}
}
#pragma endregion

#pragma region GridDebugSystem
void GridDebugSystem::init() {
	gCoordinator.getEventBus()->subscribe(this, &GridDebugSystem::onGridDebugEvent);
}

void GridDebugSystem::onGridDebugEvent(const GridDebugEvent* event) {
	displayGridOutlines = !displayGridOutlines;
}

//Doesn't work since we got rid of the grid system. Should adjust in future
void GridDebugSystem::update(float delta) {
	if (displayGridOutlines) {
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0x40);
		Vector2 start = grid->origin;
		for(int i = 0; i <= grid->gridWidth*grid->subWidth; i++) {
            if(i% grid->subWidth == 0) {
                SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF);
            }
			SDL_RenderLine(gRenderer, start.x, start.y, start.x, start.y + grid->gridHeight*grid->tileWidth);
			start.x += grid->tileWidth/grid->subWidth;
            if(i%grid->subWidth == 0) {
                SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0x40);
            }
		}
		start = grid->origin;
		for(int i = 0; i <= grid->gridHeight*grid->subWidth; i++) {
            if(i%grid->subWidth == 0) {
                SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF);
            }
			SDL_RenderLine(gRenderer, start.x, start.y, start.x+ grid->gridWidth*grid->tileWidth, start.y);
			start.y += grid->tileWidth/grid->subWidth;
            if(i%grid->subWidth== 0) {
                SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0x40);
            }
		}
	}
}
#pragma endregion

#pragma region PathFindingDebugSystem
void PathFindingDebugSystem::init() {
	gCoordinator.getEventBus()->subscribe(this, &PathFindingDebugSystem::onPathFindingDebugEvent);
}

void PathFindingDebugSystem::onPathFindingDebugEvent(const PathFindingDebugEvent* event) {
	displayPath = !displayPath;
}

void PathFindingDebugSystem::update(float dt) {
	if (displayPath) {
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);
		for (Entity e : registeredEntities) {
			Pathfinding p = gCoordinator.getComponent<Pathfinding>(e);

			//Draw a line between each pair of coordinates to form the path
			if (p.path.size() > 1) {
				for (int i = 0; i < p.path.size()-1; i++) {
					SDL_RenderLine(gRenderer, p.path[i].x, p.path[i].y, p.path[i + 1].x, p.path[i + 1].y);
				}
			}
		}
	}
}
#pragma endregion
