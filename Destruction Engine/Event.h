#pragma once
#include <vector>
class Event {
protected:
	virtual ~Event() {};
};

struct ErasureEvent : public Event {};

struct ColliderDebugEvent : public Event {};

struct GridDebugEvent : public Event {};

struct GridChangedEvent : public Event {
	std::vector<std::vector<int>>grid;

	GridChangedEvent(std::vector<std::vector<int>> g) : grid(g) {}
};