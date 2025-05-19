#include "System.h"
void System::registerEntity(Entity const &entity) {
	registeredEntities.insert(entity);
}

void System::unRegisterEntity(Entity const& entity) {
    /*for (auto it = registeredEntities.begin(); it != registeredEntities.end(); ++it) {
        Entity e = *it;
        if (e.id == entity.id) {
            registeredEntities.erase(it);
            return;
        }
    }*/
    registeredEntities.erase(entity);
}