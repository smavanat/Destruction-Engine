#pragma once
#include "System.h"
#include "BasicComponents.h"
#include "Coordinator.h"
#include "Entity.h"
#include "GridSystem.h"

extern Coordinator gCoordinator;
extern SDL_Renderer* gRenderer;

using Debug = System;

//Manager class to batch run all of the Debug tools
//Actual system management of entities etc is handled by the SystemManager still, just to make things easier
class DebugManager {
public:
	DebugManager();
	void init();
	void update(float dt);

	//Have to write this method in a header class since it is a template
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

//Highlights collider outlines
class ColliderDebugSystem : public Debug {
public:
	void init();
	void update(float dt);
	void onColliderDebugEvent(const ColliderDebugEvent* event);
private:
	bool displayColliderOutlines = false;
};

//Gets the grid outlines
class GridDebugSystem : public Debug {
public:
	void init();
	void update(float dt);
	void onGridDebugEvent(const GridDebugEvent* event);
private:
	bool displayGridOutlines = false;
};

//Shows paths
class PathFindingDebugSystem : public Debug {
public:
	void init();
	void update(float dt);
	void onPathFindingDebugEvent(const PathFindingDebugEvent* event);
private:
	bool displayPath = false;
};