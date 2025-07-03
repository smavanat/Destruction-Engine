#pragma once
#include <memory>
#include<unordered_map>
#include<iostream>
#include "System.h"
class SystemManager {
public:
	//Maybe make this take an array of entites, and then it automatically assigns the entites to the new system?
	//template<typename T> 
	//std::shared_ptr<T> registerSystem() {
	//	std::string typeName = typeid(T).name();//I don't like this trick. Find a better way of doing things

	//	assert(systemMap.find(typeName) == systemMap.end() && "Registering System more than once");

	//	//create a pointer to the system and return it so that it can be used externally
	//	auto system = std::make_shared<T>();
	//	systemMap.insert({ typeName, system });
	//	return system;
	//}

	template<typename T>
	std::shared_ptr<T> registerSystem(Signature sig) {
		std::string typeName = typeid(T).name();//I don't like this trick. Find a better way of doing things

		assert(systemMap.find(typeName) == systemMap.end() && "Registering System more than once");

		//create a pointer to the system and return it so that it can be used externally
		auto system = std::make_shared<T>();
		systemMap.insert({ typeName, system });
		signatureMap.insert({ typeName, sig });
		return system;
	}

	template<typename T>
	void setSignature(Signature signature) {
		std::string typeName = typeid(T).name();

		assert(systemMap.find(typeName) != systemMap.end() && "System used before registered.");
		// Set the signature for this system
		signatureMap.insert({ typeName, signature });
	}

	void entityDestroyed(Entity entity)
	{
		// Erase a destroyed entity from all system lists
		// mEntities is a set so no check needed
		for (auto const& pair : systemMap)
		{
			auto const& system = pair.second;

			system->unRegisterEntity(entity);
		}
	}

	void entitySignatureChanged(Entity entity, Signature entitySignature)
	{
		// Notify each system that an entity's signature changed
		for (auto const& pair : systemMap)
		{
			auto const& type = pair.first;
			auto const& system = pair.second;
			auto const& systemSignature = signatureMap[type];

			// Entity signature matches system signature - insert into set
			if ((entitySignature & systemSignature) == systemSignature)
			{
				//std::cout << "Entity " << entity.id << " matches system "
				//	<< systemSignature.mask << " with sig: " << entitySignature.mask << "\n";
				system->registerEntity(entity);
				//printf("Adding an entity to the %s system\n", type.name());
			}
			// Entity signature does not match system signature - erase from set
			else
			{
				system->unRegisterEntity(entity);
			}
		}
	}

	//Initialises all the systems stored in the manager
	void init() {
		for (auto const& pair : systemMap) {
			if (pair.second) {
				pair.second->init();
			}
		}
	}
private:
	//Maps system type to its signature.
	std::unordered_map<std::string, Signature> signatureMap;
	//Map typename to actual type. For checking if we already store this system or not
	std::unordered_map<std::string, std::shared_ptr<System>> systemMap;
};