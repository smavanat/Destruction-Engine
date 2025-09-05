#pragma once
#include <set>
#include "Entity.h"
#include "Signature.h"
class System {
public:
	System() = default;

	virtual ~System() = default;

	virtual void init() {};

	virtual void update(float dt) {};

	void registerEntity(Entity const &entity);

	void unRegisterEntity(Entity const& entity);

	std::set<Entity> registeredEntities;
};