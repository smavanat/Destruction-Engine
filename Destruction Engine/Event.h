#pragma once
class Event {
protected:
	virtual ~Event() {};
};

struct ErasureEvent : public Event {};

struct ColliderDebugEvent : public Event {};