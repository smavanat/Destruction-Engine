#include<memory>
#include<array>
#include "Component.h"
#include "ComponentArray.h"
//Class for holding and dealing with all the individual ComponentArrays
class ComponentManager {
public:
	template<typename T>
	void addComponent(Entity e, Component<T> c) {
		getComponentArray<T>()->addComponent(e, c);
	}

	template<typename T>
	void removeComponent(Entity e) {
		getComponentArray<T>()->removeComponent(e);
	}

	template<typename T>
	T& getComponent(Entity e) {
		getComponentArray<T>()->getComponent(e);
	}

	void destroyEntity(Entity e) {
		for (int i = 0; i < numComponentArrays; i++) {
			componentArrays[i]->entityDestroyed(e);
		}
	}

private:
	std::array<std::shared_ptr<IComponentArray>, MAX_COMPONENTS> componentArrays;
	int numComponentArrays = 0;
	template<typename T>
	std::shared_ptr<ComponentArray<T>> getComponentArray() {
		//The component does not have a component array assigned to it we need to make a new one
		if (GetComponentFamily<T>() >= numComponentArrays) {
			componentArrays[numComponentArrays] = std::make_shared<ComponentArray<T>>();
			return componentArrays[numComponentArrays++];
		}
		return componentArrays[GetComponentFamily<T>()];
	}
};