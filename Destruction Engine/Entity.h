#pragma once
#include "box2d/base.h"
#include "box2d/box2d.h"
//An entity is literally just an id
#pragma once
struct Entity
{
	int id;

	bool operator == (const Entity& o) const {
		return o.id == id;
	}

	bool operator < (const Entity& o) const {
		return o.id < id;
	}
};

constexpr int MAX_ENTITIES = 4096;

