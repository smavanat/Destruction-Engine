#include "Event.h"
#include "SDL3/SDL_rect.h"
#include "System.h"
#include "BasicComponents.h"
#include "Coordinator.h"
#include "Entity.h"
#include "Outline.h"
#include "GridManager.h"
#include <cstdio>
#include <memory>

extern Coordinator gCoordinator;
extern SDL_Renderer* gRenderer;
extern int scale;
extern b2WorldId worldId;
extern std::shared_ptr<GridData> grid;

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
			Transform& transform = gCoordinator.getComponent<Transform>(entity);
			Collider collider = gCoordinator.getComponent<Collider>(entity);
			
			Vector2 temp = b2Body_GetPosition(collider.colliderId);
			transform.rotation = normalizeAngle(b2Rot_GetAngle(b2Body_GetRotation(collider.colliderId)))/DEGREES_TO_RADIANS;
			transform.position = (Vector2){temp.x*metresToPixels, temp.y * metresToPixels};
		}
	}
};

//Modifies and destroys entities after they have been altered in some way
class DestructionSystem : public System {
public:
	void init() {
		gCoordinator.getEventBus()->subscribe(this, &DestructionSystem::onErasureEvent);
        gCoordinator.getEventBus()->subscribe(this, &DestructionSystem::onDestructionEvent);
	}

    void onDestructionEvent(const DestructionEvent* destr) {
        for(Entity e : registeredEntities) {
            Sprite& s = gCoordinator.getComponent<Sprite>(e);
            Transform& t = gCoordinator.getComponent<Transform>(e);
            Terrain& tr = gCoordinator.getComponent<Terrain>(e);
            SDL_FRect sRect = (SDL_FRect){getOrigin(s, t).x, getOrigin(s, t).y, static_cast<float>(s.surfacePixels->w), static_cast<float>(s.surfacePixels->h)};
            if(tr.isTerrain && isOverlapping(&sRect, t.rotation, destr->quad, destr->rot)) {
                erasePixelsRectangle(s,t, gRenderer, destr->quad->w/2, destr->quad->h/2, destr->rot, destr->quad->x, destr->quad->y);
            }
        }
        gCoordinator.getEventBus()->publish(new ErasureEvent());
    }

	void onErasureEvent(const ErasureEvent* erasure) {
		//Need to hold the removed/added entities in vectors so we don't modify the registeredEntities set as we iterate over it
		std::vector<Entity> entitiesToRemove; 
		std::vector<Sprite> spritesToAdd;
		std::vector<Transform> transformsToAdd;
		std::vector<Terrain> terrainToAdd;
		std::vector<b2BodyType> collidersToAdd;

		for (Entity entity : registeredEntities) {
			Sprite& s = gCoordinator.getComponent<Sprite>(entity);
			if (s.needsSplitting) {
				Transform& t = gCoordinator.getComponent<Transform>(entity);
				Terrain te = gCoordinator.getComponent<Terrain>(entity);
				Collider c = gCoordinator.getComponent<Collider>(entity);
				for (std::pair<Sprite, Transform> pair : splitTextureAtEdge(s, t, gRenderer)) {
					spritesToAdd.push_back(pair.first);
					transformsToAdd.push_back(pair.second);
					terrainToAdd.push_back(Terrain(te.isTerrain));
					collidersToAdd.push_back(b2Body_GetType(c.colliderId));
				}
				s.needsSplitting = false;
				entitiesToRemove.push_back(entity);
			}
		}

		for (Entity e : entitiesToRemove) {
			Transform t = gCoordinator.getComponent<Transform>(e);
			Collider c = gCoordinator.getComponent<Collider>(e);
			Sprite s = gCoordinator.getComponent<Sprite>(e);
			Terrain te = gCoordinator.getComponent<Terrain>(e);
			if(te.isTerrain) {
				Vector2 gridPosition = worldToGridPos(grid, t.position);
				intersectingSubcells(grid, &c, false, (Vector2){gridPosition.x*grid->tileWidth, gridPosition.y*grid->tileWidth});
			}
			free(s.surfacePixels);
			b2DestroyBody(c.colliderId);
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
			Collider* c = new Collider(createTexturePolygon(temprdpPoints, width, worldId, collidersToAdd[i], transformsToAdd[i]), POLYGON);
			if(terrainToAdd[i].isTerrain) {
				Vector2 gridPosition = worldToGridPos(grid, transformsToAdd[i].position);
				intersectingSubcells(grid, c, true, (Vector2){gridPosition.x*grid->tileWidth, gridPosition.y*grid->tileWidth});
			}
			gCoordinator.addComponent(e, *c);
			gCoordinator.addComponent(e, terrainToAdd[i]);
		}
		spritesToAdd.clear();
		transformsToAdd.clear();
		terrainToAdd.clear();

		entitiesToRemove.clear();
	}

	void update(float dt) {
		Input* in = gCoordinator.getInput();
        if(gCoordinator.state != THIRD) {
            if (in->leftMouseButtonDown) {//Checking that lmb is activated
                if(in->mouseX < 0 || in->mouseX > grid->gridWidth*grid->tileWidth
                    || in->mouseY < 0 || in->mouseY > grid->gridHeight*grid->tileWidth)
                        return;
                for (Entity entity : registeredEntities) {
                    Sprite &s = gCoordinator.getComponent<Sprite>(entity);
                    Transform t = gCoordinator.getComponent<Transform>(entity);
                    Vector2 temp = {in->mouseX, in->mouseY};
                    Vector2 rotated = rotateAboutPoint(&temp, &t.position, -t.rotation, false);

                    //This is the correct version for doing outside
                    if((rotated.x + scale >= getOrigin(s, t).x && rotated.x + scale < getOrigin(s,t ).x + s.surfacePixels->w) //West
                        || (rotated.y + scale >= getOrigin(s, t).y && rotated.x + scale < getOrigin(s,t ).y + s.surfacePixels->h) //North
                        || (rotated.x - scale >= getOrigin(s, t).x && rotated.x - scale < getOrigin(s,t ).x + s.surfacePixels->w) //East
                        || (rotated.y - scale >= getOrigin(s, t).y && rotated.x - scale < getOrigin(s,t ).y + s.surfacePixels->h) //South
                    ) {
                        erasePixels(s, t, gRenderer, scale, in->mouseX, in->mouseY);
                    }
                }
            }
        }
	}
};

class UISystem : public System{
public: 
    void init() {
		gCoordinator.getEventBus()->subscribe(this, &UISystem::onClickedEvent);
    }

    void onClickedEvent(const ClickedEvent* event) {
        for(Entity e : registeredEntities) {
            Button b = gCoordinator.getComponent<Button>(e);
            if(event->pos.x >= b.area->x && event->pos.x < b.area->x + b.area->w
                && event->pos.y >= b.area->y && event->pos.y < b.area->y + b.area->h) {
                //Do something
                printf("Clicked a button\n");
                gCoordinator.state = b.state;
            }
        }
    }

    void update(float dt) {}
};
