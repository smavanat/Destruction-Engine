#pragma once
struct ComponentCounter {
	static int counter;
};

//int ComponentCounter::counter = 0; //Need to declare this somewhere else

template<typename CType>
struct Component {
	static inline int componentTypeId() {
		static int id = ComponentCounter::counter++;
		return id;
	}
};

template <typename C>
static int GetComponentFamily() {
	return Component<typename std::remove_const<C>::type>::componentTypeId();
}

//const int MAX_COMPONENTS = 64;
constexpr int MAX_COMPONENTS = 64;