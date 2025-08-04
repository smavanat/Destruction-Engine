#include "System.h"
#include "BasicComponents.h"
#include "Coordinator.h"
#include "Entity.h"
#include "Outline.h"
#include "GridManager.h"

extern Coordinator gCoordinator;
extern SDL_Renderer* gRenderer;
extern int scale;
extern b2WorldId worldId;
extern GridSystemManager gGridManager;

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

			render(sprite, transform, gRenderer);
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
			
			b2Vec2 temp = b2Body_GetPosition(collider.colliderId);
			transform.position = convertToVec2(&temp);
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
		std::vector<Transform> transformsToAdd;

		for (Entity entity : registeredEntities) {
			Sprite& s = gCoordinator.getComponent<Sprite>(entity);
			Transform& t = gCoordinator.getComponent<Transform>(entity);
			if (s.needsSplitting) {
				for (std::pair<Sprite, Transform> pair : splitTextureAtEdge(s, t, gRenderer)) {
					spritesToAdd.push_back(pair.first);
					transformsToAdd.push_back(pair.second);
				}
				s.needsSplitting = false;
				entitiesToRemove.push_back(entity);
			}
		}

		for (Entity e : entitiesToRemove) {
			Transform t = gCoordinator.getComponent<Transform>(e);
			Collider c = gCoordinator.getComponent<Collider>(e);
			intersectingSubcells(gGridManager.grid, worldToGridIndex(gGridManager.grid, t.position), &c, false);
			gCoordinator.destroyEntity(e);
		}

		for (int i = 0; i < spritesToAdd.size(); i++) {
			std::vector<int> tempPoints = marchingSquares(spritesToAdd[i]);
			std::vector<int> temprdpPoints;
			//Position at size()-2 is where 0 is stored. This will give us the 
			//straight line that we want. If we add origin at end as well it messes up partition so don't do that.
			temprdpPoints.push_back(tempPoints[tempPoints.size() - 2]);
			int width = spritesToAdd[i].surfacePixels->w;
			rdp(0, tempPoints.size() - 1, 3, width, tempPoints, temprdpPoints);
			Entity e = gCoordinator.createEntity();
			gCoordinator.addComponent(e, transformsToAdd[i]);
			gCoordinator.addComponent(e, spritesToAdd[i]);
			Collider* c = new Collider(createTexturePolygon(temprdpPoints, width, worldId, spritesToAdd[i], transformsToAdd[i]));
			intersectingSubcells(gGridManager.grid, worldToGridIndex(gGridManager.grid, transformsToAdd[i].position), c, true);
			gCoordinator.addComponent(e, *c);
		}
		spritesToAdd.clear();
		transformsToAdd.clear();

		entitiesToRemove.clear();
	}

	void update(float dt) {
		Input* in = gCoordinator.getInput();
		if (in->leftMouseButtonDown) {//Checking that lmb is activated
			for (Entity entity : registeredEntities) {
				Sprite &s = gCoordinator.getComponent<Sprite>(entity);
				Transform t = gCoordinator.getComponent<Transform>(entity);
				Vector2 temp = {in->mouseX, in->mouseY};
				Vector2 rotated = rotateAboutPoint(&temp, &t.position, -t.rotation, false);
				if (rotated.x >= getOrigin(s, t).x && rotated.x < getOrigin(s,t ).x + s.surfacePixels->w &&
					rotated.y < getOrigin(s, t).y + s.surfacePixels->h && rotated.y >= getOrigin(s, t).y
					) {
					erasePixels(s, t, gRenderer, scale, in->mouseX, in->mouseY);
				}
			}
		}
	}
};
