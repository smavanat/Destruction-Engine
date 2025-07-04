#pragma once
#include <vector>
/*
* This file holds all of the different types of events
*/

//Base event class
class Event {
protected:
	virtual ~Event() {};
};

//These events are all really basic and just exist to be fired to tell some system that something has happened,
//they don't send any data
struct ErasureEvent : public Event {};

struct ColliderDebugEvent : public Event {};

struct GridDebugEvent : public Event {};

struct PathFindingDebugEvent : public Event {};

//This event is used to update the pathfinding system with a new grid from the Grid system
struct GridChangedEvent : public Event {
	std::vector<std::vector<int>>grid;

	GridChangedEvent(std::vector<std::vector<int>> g) : grid(g) {}
};