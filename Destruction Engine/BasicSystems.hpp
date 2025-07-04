#include "System.h"
#include "BasicComponents.h"
#include "Coordinator.h"
#include "Entity.h"
#include "Outline.hpp"

extern Coordinator gCoordinator;
extern SDL_Renderer* gRenderer;
extern int scale;
extern b2WorldId worldId;

//Renders Sprites
class RenderSystem : public System {
public:
	void init() {
		
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

//Currently not used, but ideally would update an entity's transform after physics has been applied
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

//Modifies and destroys entities after they have been altered in some way
class DestructionSystem : public System {
public:
	void init() {
		gCoordinator.getEventBus()->subscribe(this, &DestructionSystem::onErasureEvent);
	}

	void onErasureEvent(const ErasureEvent* erasure) {
		//Need to hold the removed/added entities in vectors so we don't modify the registeredEntities set as we iterate over it
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
		if (in->leftMouseButtonDown) {//Checking that lmb is activated
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