#include "System.h"
#include "BasicComponents.h"
#include "Coordinator.h"
#include "Entity.h"
#include "Outline.hpp"

extern Coordinator gCoordinator;
extern SDL_Renderer* gRenderer;
extern int scale;
extern b2WorldId worldId;
class RenderSystem : public System {
public:
	void init() {
		Signature sig;
		sig.addComponent<Transform>();
		sig.addComponent<Sprite>();
		gCoordinator.setSystemSignature<RenderSystem>(sig);
	}

	void update(float dt) {
		//printf("Number of registered entities: %i\n", registeredEntities.size());
		for (Entity entity : registeredEntities) {
			Transform transform = gCoordinator.getComponent<Transform>(entity);
			Sprite sprite = gCoordinator.getComponent<Sprite>(entity);

			sprite.centre = transform.position;
			sprite.angle = transform.rotation;
			s_render(sprite, gRenderer);
		}
	}
};

class TransformSystem : public System {
public:
	void init() {
		
	}

	void update(float dt) {
		for (Entity entity : registeredEntities) {
			Transform transform = gCoordinator.getComponent<Transform>(entity);
			Collider collider = gCoordinator.getComponent<Collider>(entity);

			transform.position = b2Body_GetPosition(collider.colliderId);
			transform.rotation = normalizeAngle(b2Rot_GetAngle(b2Body_GetRotation(collider.colliderId)))/DEGREES_TO_RADIANS;
		}
	}
};

class DestructionSystem : public System {
public:
	void init() {
		Signature sig;
		sig.addComponent<Sprite>();
		sig.addComponent<Collider>();
		gCoordinator.setSystemSignature<DestructionSystem>(sig);
		gCoordinator.getEventBus()->subscribe(this, &DestructionSystem::onErasureEvent);
	}

	void onErasureEvent(const ErasureEvent* erasure) {
			std::vector<Entity> entitiesToRemove;
			std::vector<Sprite> spritesToAdd;

			for (Entity entity : registeredEntities) {
				Sprite& s = gCoordinator.getComponent<Sprite>(entity);
				if (s.needsSplitting) {
					for (Sprite &newSprite : s_splitTextureAtEdge(s, gRenderer)) {
						spritesToAdd.push_back(newSprite);
					}
					s.needsSplitting = false;
					entitiesToRemove.push_back(entity);
				}
			}

			for (Sprite s : spritesToAdd) {
				std::vector<int> tempPoints = s_marchingSquares(s);

				std::vector<int> temprdpPoints;
				//Position at size()-2 is where 0 is stored. This will give us the 
				//straight line that we want. If we add origin at end as well it messes up partition so don't do that.
				temprdpPoints.push_back(tempPoints[tempPoints.size() - 2]);
				rdp(0, tempPoints.size() - 1, 3, s.width, tempPoints, temprdpPoints);
				Entity e = gCoordinator.createEntity();
				gCoordinator.addComponent(e, Transform(s.centre, s.angle));
				gCoordinator.addComponent(e, s);
				gCoordinator.addComponent(e, Collider(s_createTexturePolygon(temprdpPoints, s.width, worldId, s)));
			}
			spritesToAdd.clear();

			for (Entity e : entitiesToRemove) {
				gCoordinator.destroyEntity(e);
			}
			entitiesToRemove.clear();
	}

	void update(float dt) {
		Input* in = gCoordinator.getInput();
		if (in->leftMouseButtonDown) {
			for (Entity entity : registeredEntities) {
				Sprite &s = gCoordinator.getComponent<Sprite>(entity);
				Vector2 rotated = rotateAboutPoint(newVector2(in->mouseX, in->mouseY), s.centre, -s.angle, false);
				if (rotated.x >= s_getOrigin(s).x && rotated.x < s_getOrigin(s).x + s.width &&
					rotated.y < s_getOrigin(s).y + s.height && rotated.y >= s_getOrigin(s).y
					) {
					s_erasePixels(s, gRenderer, scale, in->mouseX, in->mouseY);
				}
			}
		}
	}
};

class DebugSystem : public System {
public:
	void init() {
		Signature sig;
		sig.addComponent<Collider>();
		gCoordinator.setSystemSignature<DebugSystem>(sig);
		gCoordinator.getEventBus()->subscribe(this, &DebugSystem::onColliderDebugEvent);
	}

	void onColliderDebugEvent(const ColliderDebugEvent* event) {
		displayColliderOutlines = !displayColliderOutlines;
	}

	void update(float dt) {
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
private:
	bool displayColliderOutlines = false;
};