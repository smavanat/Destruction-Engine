#pragma once
#include <type_traits>
//Holds the component "family" of the newest component. Helps us easily differentiate different components
struct ComponentCounter {
	static int counter;
};

//Basic class from which all other components inherit from
template<typename CType>
struct Component {
	static inline int componentTypeId() {
		static int id = ComponentCounter::counter++;
		return id;
	}
	Component() = default;
};

template <typename C>
static int GetComponentFamily() {
	return Component<typename std::remove_const<C>::type>::componentTypeId();
}

constexpr int MAX_COMPONENTS = 64;