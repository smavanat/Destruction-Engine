#include "Debug.h"
#pragma region DebugManager
DebugManager::DebugManager() {
	Signature sig;
	sig.addComponent<Collider>();
	std::shared_ptr<ColliderDebugSystem> cPtr = gCoordinator.addSystem<ColliderDebugSystem>(sig);
	Signature sig2;
	sig2.addComponent<Transform>();
	sig2.addComponent<Walkable>();
	std::shared_ptr<GridDebugSystem> gPtr= gCoordinator.addSystem<GridDebugSystem>(sig2);
	registerDebugSystem<ColliderDebugSystem>(cPtr);
	registerDebugSystem<GridDebugSystem>(gPtr);
}

//Initialises all the systems stored in the manager
void DebugManager::init() {
	
}

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
	/*Signature sig;
	sig.addComponent<Collider>();
	gCoordinator.setSystemSignature<ColliderDebugSystem>(sig);*/
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
			int shapeCount = b2Body_GetShapeCount(c.colliderId);
			b2Vec2 colliderPosition = b2Body_GetPosition(c.colliderId);
			b2ShapeId* colliderShapes = new b2ShapeId[shapeCount];
			b2Body_GetShapes(c.colliderId, colliderShapes, shapeCount);
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
			Transform t = gCoordinator.getComponent<Transform>(e);
			SDL_FRect tileRect = { t.position.x - TILE_WIDTH / 2, t.position.y - TILE_HEIGHT / 2, TILE_WIDTH, TILE_HEIGHT };
			SDL_RenderRect(gRenderer, &tileRect);
		}
	}
}
#pragma endregion
