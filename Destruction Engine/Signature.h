#pragma once
#include<bitset>
#include "Component.h"
//The signature class is for storing a bitwise representation of the components that an entity contains
struct Signature {
	std::bitset<MAX_COMPONENTS> mask = 0; //The actual mask containing the components

	//Adding a component to the mask
	template <typename ComponentType>
	void addComponent() {
		//Just left shift the one by the amount of the component family and perform an OR with the current mask to update it
		mask |= (1 << GetComponentFamily<ComponentType>()); 
	}

	//Just left shift the one by the amount of the component family and perform an AND with the current mask to update it
	template <typename ComponentType>
	void removeComponent() {
		mask &= ~(1 << GetComponentFamily<ComponentType>());
	}

	std::bitset<MAX_COMPONENTS> operator & (const Signature& o) const {
		return mask & o.mask;
	}

	bool operator == (const Signature& o) const {
		return mask == o.mask;
	}
};