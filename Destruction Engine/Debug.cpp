#include "Debug.h"
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
		sig.addComponent<Transform>();
		sig.addComponent<Walkable>();
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
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);//Setting the line colour to black
		//For drawing "normal" colliders. Assuming that all colliders are made up of triangles.
		//Triangles I think are the way forward, need to make it so all colliders are made up of triangle 
		//polygons. It just makes things easier.
		for (Entity entity : registeredEntities) {
			Collider c = gCoordinator.getComponent<Collider>(entity);

			//This is the unfortunately very convoluted process of getting the shapes that make up a 
			//box2D body
			int shapeCount = b2Body_GetShapeCount(c.colliderId);
			b2Vec2 colliderPosition = b2Body_GetPosition(c.colliderId);
			b2ShapeId* colliderShapes = new b2ShapeId[shapeCount];
			b2Body_GetShapes(c.colliderId, colliderShapes, shapeCount);

			//There is unfortunately no way to get the number of vertices a shape has, 
			//since the arrays are always of size 8 (max number of vertices that a shape has)
			//This is another reason for using only triangles, it standardises everything so that
			//we don't need to use any convoluted process to figure out how many vertices are in a shape
			for (int j = 0; j < shapeCount; j++) {
				b2Vec2* colliderVertices = b2Shape_GetPolygon(colliderShapes[j]).vertices;
				for (int k = 0; k < 3; k++) {
					rotateTranslate(colliderVertices[k], b2Rot_GetAngle(b2Body_GetRotation(c.colliderId)));
				}
				for (int k = 0; k < 3; k++) {
					SDL_RenderLine(gRenderer, ((colliderVertices[k].x + colliderPosition.x) * metresToPixels), ((colliderVertices[k].y + colliderPosition.y) * metresToPixels),
						((colliderVertices[(k + 1) > 2 ? 0 : (k + 1)].x + colliderPosition.x) * metresToPixels), ((colliderVertices[(k + 1) > 2 ? 0 : (k + 1)].y + colliderPosition.y) * metresToPixels));
				}
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

void GridDebugSystem::update(float delta) {
	if (displayGridOutlines) {
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF);
		for (Entity e : registeredEntities) {
			//Just draw a box around the position of the tile (always in the center of the tile)
			Transform t = gCoordinator.getComponent<Transform>(e);
			SDL_FRect tileRect = { t.position.x - TILE_WIDTH / 2, t.position.y - TILE_HEIGHT / 2, TILE_WIDTH, TILE_HEIGHT };
			SDL_RenderRect(gRenderer, &tileRect);
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