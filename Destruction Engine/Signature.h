#pragma once
#include<bitset>
#include "Component.h"
//The signature class is for storing a bitwise representation of the components that an entity contains
struct Signature {
	std::bitset<MAX_COMPONENTS> mask{}; //The actual mask containing the components

	template <typename ComponentType>
	void addComponent() {
		mask.set(GetComponentFamily<ComponentType>());
	}

	template <typename ComponentType>
	void removeComponent() {
		mask.reset(GetComponentFamily<ComponentType>());
	}

	Signature operator&(const Signature& o) const {
		Signature result;
		result.mask = mask & o.mask;
		return result;
	}

	bool operator == (const Signature& o) const {
		return mask == o.mask;
	}
};