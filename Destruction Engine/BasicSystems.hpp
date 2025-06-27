#include "System.h"
#include "BasicComponents.h"
#include "Coordinator.h"
#include "Entity.h"
extern Coordinator gCoordinator;
extern SDL_Renderer* gRenderer;
class RenderSystem : public System {
public:
	void init() {}

	void update(float dt) {
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);
		for (Entity entity : registeredEntities) {
			Transform transform = gCoordinator.getComponent<Transform>(entity);
			Sprite sprite = gCoordinator.getComponent<Sprite>(entity);

			sprite.centre = transform.position;
			sprite.angle = transform.rotation;
			s_render(sprite, gRenderer);
		}
		SDL_RenderPresent(gRenderer);
	}
};

class TransformSystem : public System {
public:
	void init() {}

	void update(float dt) {
		for (Entity entity : registeredEntities) {
			Transform transform = gCoordinator.getComponent<Transform>(entity);
			Collider collider = gCoordinator.getComponent<Collider>(entity);

			transform.position = b2Body_GetPosition(collider.colliderId);
			transform.rotation = normalizeAngle(b2Rot_GetAngle(b2Body_GetRotation(collider.colliderId)))/DEGREES_TO_RADIANS;
		}
	}
};