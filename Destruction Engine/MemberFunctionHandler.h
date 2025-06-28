#pragma once
#include "Event.h"
class HandlerFunctionBase
{
public:
	virtual ~HandlerFunctionBase() {};
	void exec(const Event* event) { call(event); }

private:
	virtual void call(const Event*) = 0;
};

template < class T, class EventT > 
class MemberFunctionHandler : public HandlerFunctionBase
{
public:
	typedef void (T::* MemberFunc)(const EventT*);
	MemberFunctionHandler(T* instance, MemberFunc memFn) : _instance(instance), _function(memFn) {};
	void call(const Event* event) override
	{
		(_instance->*_function)(static_cast<const EventT*>(event));
	}

private:
	T* _instance;
	MemberFunc _function;
};