#include "System.h"
void System::registerEntity(Entity const &entity) {
	registeredEntities.insert(entity);
}

void System::unRegisterEntity(Entity const& entity) {
    registeredEntities.erase(entity);
}