#pragma once
#include<queue>
#include<array>
#include<cassert>
#include "Entity.h"
#include "Signature.h"

//Class for managing and recycling all the entities in this system.
class EntityManager {
public:
	EntityManager() {
		for (int i = 0; i < MAX_ENTITIES; i++) {
			availableEntities.push({ i });
		}
		usedEntityCount = 0;
	}

	Entity createEntity() {
		assert(usedEntityCount < MAX_ENTITIES && "Too many entities in existence");

		Entity id = availableEntities.front();
		availableEntities.pop();//For some stupid reason pop() doesn't return a value
		usedEntityCount++;

		return id;
	}

	void destroyEntity(Entity entity) {
		assert(entity.id < MAX_ENTITIES && "Entity out of range");

		signatures[entity.id].mask.reset();//Setting the value of the signature to its default state
		availableEntities.push(entity);
		usedEntityCount--;
	}

	//Need to figure out how to dynamically update signatures when a singular component is added rather than just calling this function all the time
	void setSignature(Entity e, Signature s) {
		assert(e.id < MAX_ENTITIES && "Entity out of range");

		signatures[e.id] = s;
	}

	Signature getSignature(Entity e) {
		assert(e.id < MAX_ENTITIES && "Entity out of range");

		return signatures[e.id];
	}

private:
	std::queue<Entity> availableEntities;
	std::array<Signature, MAX_ENTITIES> signatures;
	uint32_t usedEntityCount;
};