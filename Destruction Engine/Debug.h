#pragma once
#include "System.h"
#include "BasicComponents.h"
#include "Coordinator.h"
#include "Entity.h"
#include "Outline.hpp"
#include "GridSystem.h"

extern Coordinator gCoordinator;
extern SDL_Renderer* gRenderer;

using Debug = System;

class DebugManager {
public:
	DebugManager();
	void init();
	void update(float dt);

	//Allows users to add their own debug systems. All in-built ones are just added in init
	template<typename T>
	std::shared_ptr<T> registerDebugSystem(std::shared_ptr<T> dPointer) {
		std::string typeName = typeid(T).name();//I don't like this trick. Find a better way of doing things

		assert(debugMap.find(typeName) == debugMap.end() && "Registering System more than once");

		//create a pointer to the system and return it so that it can be used externally
		auto debug = std::make_shared<T>();
		debugMap.insert({ typeName, dPointer });
		return debug;
	}
private:
	//Map typename to actual type. For checking if we already store this system or not
	std::unordered_map<std::string, std::shared_ptr<Debug>> debugMap;
};

class ColliderDebugSystem : public Debug {
public:
	void init();
	void update(float dt);
	void onColliderDebugEvent(const ColliderDebugEvent* event);
private:
	bool displayColliderOutlines = false;
};

class GridDebugSystem : public Debug {
public:
	void init();
	void update(float dt);
};