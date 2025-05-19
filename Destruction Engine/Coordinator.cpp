#include "Coordinator.h"
Coordinator::Coordinator(std::unique_ptr<SystemManager> sManager, std::unique_ptr<ComponentManager> cManager, std::unique_ptr<EntityManager> eManager) {
	sysManager = std::move(sManager);
	compManager = std::move(cManager);
	entManager = std::move(eManager);
}

void Coordinator::init() {
	sysManager->init();
}

//Create a new entity
Entity Coordinator::createEntity() {
	return entManager->createEntity();
}

//Destroy an entity and all of its components
void Coordinator::destroyEntity(Entity& e) {
	entManager->destroyEntity(e);
	compManager->destroyEntity(e);
	sysManager->entityDestroyed(e);
}

//Add a new system to the engine. Should happen before init
template <typename T>
std::shared_ptr<T> Coordinator::addSystem() {
	sysManager->registerSystem<T>();
}

template <typename T>
void Coordinator::setSystemSignature(Signature s) {
	sysManager->setSignature<T>(s);
}

//Add a new component to an entity
template<typename T>
void Coordinator::addComponent(Entity& e, T&& c) {
	compManager->addComponent(e, c);
	auto sig = entManager->getSignature(e);
	sig.addComponent(c);
	sysManager->entitySignatureChanged(e, sig);
}

//Get a component from an entity
template<typename T>
T Coordinator::getComponent(Entity& e) {
	return compManager->getComponent(e);
}

//Remove a component from an entity
template<typename T>
void Coordinator::removeComponent(Entity& e) {
	compManager->removeComponent(e);
	auto sig = entManager->getSignature(e);
	sig.removeComponent(c);
	sysManager->entitySignatureChanged(e, sig);
}