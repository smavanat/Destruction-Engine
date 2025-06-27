#pragma once
#include<memory>
#include "ComponentManager.h"
#include "EntityManager.h"
#include "SystemManager.h"
class Coordinator {
public:
	Coordinator() {
		sysManager = std::make_unique<SystemManager>();
		compManager = std::make_unique<ComponentManager>();
		entManager = std::make_unique<EntityManager>();
	}

	//Called on engine init
	void init() {
		sysManager->init();
	}

	//Create a new entity
	Entity createEntity() {
		return entManager->createEntity();
	}

	//Destroy an entity and all of its components
	void destroyEntity(Entity& e) {
		entManager->destroyEntity(e);
		compManager->destroyEntity(e);
		sysManager->entityDestroyed(e);
	}

	//Add a new system to the engine. Should happen before init
	template <typename T>
	std::shared_ptr<T> addSystem() {
		return sysManager->registerSystem<T>();
	}

	//Set a system's signature
	template <typename T>
	void setSystemSignature(Signature s) {
		sysManager->setSignature<T>(s);
	}

	//Add a new component to an entity
	template<typename T>
	void addComponent(Entity& e, T&& c) {
		compManager->addComponent(e, c);
		auto sig = entManager->getSignature(e);
		std::cout << "Entity Signature: " << sig.mask << "\n";
		sig.addComponent<T>();
		entManager->setSignature(e, sig);
		sysManager->entitySignatureChanged(e, sig);
	}

	//Get a component from an entity
	template<typename T>
	T& getComponent(Entity& e) {
		return compManager->getComponent<T>(e);
	}

	//Remove a component from an entity
	template<typename T>
	void removeComponent(Entity& e, T&& c) {
		compManager->removeComponent(e);
		auto sig = entManager->getSignature(e);
		sig.removeComponent(c);
		sysManager->entitySignatureChanged(e, sig);
	}

private:
	std::unique_ptr<SystemManager> sysManager;
	std::unique_ptr<ComponentManager> compManager;
	std::unique_ptr<EntityManager> entManager;
};

