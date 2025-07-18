#pragma once
#include <list>
#include <map>
#include <typeindex>
#include "MemberFunctionHandler.h"
/*
* The event bus is used to make subscribing and publishing events easier and more centralised
* Based on this forum post: https://www.gamedev.net/articles/programming/general-and-gameplay-programming/effective-event-handling-in-c-r2459/
*/
typedef std::list<HandlerFunctionBase*> HandlerList;
class EventBus {
public:
	template <typename EventT>
	void publish(EventT* event) {
		HandlerList* handlers = subscribers[typeid(EventT)];

		if (handlers == nullptr) return;

		for (auto& handler : *handlers) {
			if (handler != nullptr) {
				handler->exec(event);
			}
		}
	}

	template<class T, class EventT>
	void subscribe(T* instance, void(T::* memberFunction)(const EventT*)) {
		HandlerList* handlers = subscribers[typeid(EventT)];

		if (handlers == nullptr) {
			handlers = new HandlerList();
			subscribers[typeid(EventT)] = handlers;
		}

		handlers->push_back(new MemberFunctionHandler<T, EventT>(instance, memberFunction));
	}
private:
	std::map<std::type_index, HandlerList*> subscribers;
};