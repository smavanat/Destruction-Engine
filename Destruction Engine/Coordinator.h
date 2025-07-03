#pragma once
#include<memory>
#include<type_traits>
#include "ComponentManager.h"
#include "EntityManager.h"
#include "SystemManager.h"
#include "EventBus.h"
#include "Input.h"

class Coordinator {
public:
	Coordinator() {
		sysManager = std::make_unique<SystemManager>();
		compManager = std::make_unique<ComponentManager>();
		entManager = std::make_unique<EntityManager>();
		eBus = std::make_unique<EventBus>();
		input = std::make_unique<Input>();
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
	std::shared_ptr<T> addSystem(Signature sig) {
		return sysManager->registerSystem<T>(sig);
	}

	//Set a system's signature
	template <typename T>
	void setSystemSignature(Signature s) {
		sysManager->setSignature<T>(s);
	}

	//Add a new component to an entity
	template<typename T>
	void addComponent(Entity& e, T&& c) {
		// removes reference and const. 
		// This is because the signature will treat references as different types and so produce a difference 
		// signature when adding (for example) Sprite and Sprite&.
		using CleanT = std::decay_t<T>;  

		compManager->addComponent<CleanT>(e, std::forward<T>(c));
		auto sig = entManager->getSignature(e);
		sig.addComponent<CleanT>();
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

	EventBus* getEventBus() {
		return eBus.get();
	}

	Input* getInput() {
		return input.get();
	}

	Signature getSignature(Entity e) {
		return entManager->getSignature(e);
	}

private:
	std::unique_ptr<SystemManager> sysManager;
	std::unique_ptr<ComponentManager> compManager;
	std::unique_ptr<EntityManager> entManager;
	std::unique_ptr<EventBus> eBus;
	std::unique_ptr<Input> input;
};

