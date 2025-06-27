#pragma once
#include "Event.h"

class HandlerFunctionBase {
public: 
	virtual ~HandlerFunctionBase() {};
	void exec(const Event* event) { call(event); }
private:
	virtual void call(const Event*) = 0;
};

template<class T, class EventT>
class MemberFunctionHandler : public HandlerFunctionBase {
public:
	typedef void (T::* MemberFunction)(EventT*)
	MemberFunctionHandler(T* instance, MemberFunction memFn) : instance{ instance }, memberFunction{ memFn } {};
	void call(const Event* event) {
		(instance -> * memberFunction)(static_cast<EventT*>(event));
	}
private:
	T* instance;
	MemberFunction MemberFunction;
};