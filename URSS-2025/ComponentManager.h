#include<memory>
#include<array>
#include "Component.h"
#include "ComponentArray.h"
//Class for holding and dealing with all the individual ComponentArrays
class ComponentManager {
public:
	template<typename T>
	void addComponent(Entity e, T c) {
		getComponentArray<T>()->addComponent(e, c);
	}

	template<typename T>
	void removeComponent(Entity e) {
		getComponentArray<T>()->removeComponent(e);
	}

	template<typename T>
	T& getComponent(Entity e) {
		return getComponentArray<T>()->getComponent(e);
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
		const std::size_t index = GetComponentFamily<T>();

		if (!componentArrays[index]) {
			componentArrays[index] = std::make_shared<ComponentArray<T>>();
		}

		return std::static_pointer_cast<ComponentArray<T>>(componentArrays[index]);
	}
};