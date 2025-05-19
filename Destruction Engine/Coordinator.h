#pragma once
#include<memory>
#include "ComponentManager.h"
#include "EntityManager.h"
#include "SystemManager.h"
class Coordinator {
public:
	Coordinator(std::unique_ptr<SystemManager> sManager, std::unique_ptr<ComponentManager> cManager, std::unique_ptr<EntityManager> eManager);

	//Called on engine init
	void init();

	//Create a new entity
	Entity createEntity();

	//Destroy an entity and all of its components
	void destroyEntity(Entity& e);

	//Add a new system to the engine. Should happen before init
	template <typename T>
	std::shared_ptr<T> addSystem();

	//Set a system's signature
	template <typename T>
	void setSystemSignature(Signature s);

	//Add a new component to an entity
	template<typename T>
	void addComponent(Entity& e, T && c);

	//Get a component from an entity
	template<typename T>
	T getComponent(Entity& e);

	//Remove a component from an entity
	template<typename T>
	void removeComponent(Entity& e);

private:
	std::unique_ptr<SystemManager> sysManager;
	std::unique_ptr<ComponentManager> compManager;
	std::unique_ptr<EntityManager> entManager;
};

