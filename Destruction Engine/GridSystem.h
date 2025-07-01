#pragma once
#include "Coordinator.h"
class GridSystem
{
};

struct Grid : public Component<Grid> {
	const int TILE_WIDTH;
	const int TILE_HEIGHT;
	const int GRID_WIDTH_IN_TILES;
	const int GRID_HEIGHT_IN_TILES;
};