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

//Need to create a hash implementation for entity otherwise it won't work with unordered_map
namespace std {
	template<>
	struct hash<Entity> {
		std::size_t operator()(const Entity& e) const {
			return std::hash<uint32_t>()(e.id);
		}
	};
}

constexpr int MAX_ENTITIES = 4096;

