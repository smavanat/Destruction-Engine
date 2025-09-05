#include <unordered_map>
#include <cassert>
#include "Entity.h"

/*
* This file contains the classes that hold all of the same type of component in a singular dynamic array for 
* efficient memory memory access
*/

//Interface for implementing Component Array class
class IComponentArray {
public:
	virtual ~IComponentArray() = default;
	virtual void entityDestroyed(Entity e) = 0;
};
template <typename T>
class ComponentArray : public IComponentArray {
public:
	//Adds a component to the data array
	bool addComponent(Entity e, T component) {
		//If the array already contains a component related to this entity, return false
		if (entityToIndexMap.find(e) != entityToIndexMap.end()) return false;
		
		//Add the component to the end of the list and update the maps with the new kvp
		entityToIndexMap[e] = size;
		indexToEntityMap[size] = e;
		componentArray[size] = component;
		size++;//Increment size
		return true;
	}

	//Removes a component from the data array
	bool removeComponent(Entity e) {
		//If the array does not contain any components related to this entity, return false;
		if (entityToIndexMap.find(e) == entityToIndexMap.end()) return false;

		//Replace component to be removed with last element in the array and set the last element to null
		int removalIndex = entityToIndexMap[e];
		componentArray[removalIndex] = componentArray[size - 1];
		//componentArray[size - 1] = nullptr;

		//Need to update the mappings to reflect the moved entity
		Entity replacingEntity = indexToEntityMap[size - 1];//Need to get this value before any operations otherwise it will be overriden later
		entityToIndexMap[replacingEntity] = removalIndex;
		indexToEntityMap[removalIndex] = replacingEntity;

		//Erase the remvoed entity
		entityToIndexMap.erase(e);
		indexToEntityMap.erase(size - 1);
		size--;
		return true;
	}

	//Retrieves the value of a component given the entity it is attached to 
	T& getComponent(Entity e) {
		assert(entityToIndexMap.find(e) != entityToIndexMap.end() && "Retrieving non-existent component.");
		return componentArray[entityToIndexMap[e]];
	}

	//Flag to remove a component if the entity is destroyed
	void entityDestroyed(Entity e) {
		if (entityToIndexMap.find(e) != entityToIndexMap.end()) {
			removeComponent(e);
		}
	}

private:
	//Need to have both index-to-entity map and entity-to-index map so that we can represent a two-way relation.
	//Unfortunately takes up more space, but will reduce runtime operations.
	std::unordered_map<int, Entity> indexToEntityMap;
	std::unordered_map<Entity, int, EntityHasher> entityToIndexMap;
	std::array<T, MAX_ENTITIES> componentArray;
	int size = 0;
};