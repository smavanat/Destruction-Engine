#include "GridData.h"

Vector2 gridToWorldPos(std::shared_ptr<GridData> g, Vector2 gridPos) {
    return Vector2{
        gridPos.x * g->tileWidth + (g->tileWidth / 2.0f),
        gridPos.x * g->tileHeight + (g->tileHeight / 2.0f)
    };
}

Vector2 worldToGridPos(std::shared_ptr<GridData> g, Vector2 worldPos) {
    return Vector2 (
        static_cast<int>(floor(worldPos.x / (g->tileWidth / 2))),
        static_cast<int>(floor(worldPos.y / (g->tileHeight / 2)))
    );
}

int toIndex(std::shared_ptr<GridData> g, Vector2 gridPos) {
    return gridPos.x + gridPos.y * g->gridWidth;
}
bool inBounds(std::shared_ptr<GridData> g, Vector2 gridPos) {
    return gridPos.x >= 0 && gridPos.y >= 0 && gridPos.x < g->gridWidth && gridPos.y < g->gridHeight;
}

bool isDirectionWalkable(const TileData& t, Direction8 d, int s) {
    switch (d) {
        case NW:
            if (t.exitable[N_4] && t.exitable[W_4]) {
                for (int i = 0; i < s; i++) {
                    for (int j = 0; j < s; j++) {
                        if (t.subcells[(i * 4) + j] == 1)
                            return false;
                    }
                }
                return true;
            }   
            return false;
        case N:
            if (t.exitable[N_4]) {
                for (int i = 0; i < 4 - s; i++) {
                    if (t.subcells[i] == 0) {
                        bool pathable = true;
                        for (int j = 1; j < s; j++) {
                            if (t.subcells[i+j] == 1) {
                                pathable = false;
                                break;
                            }
                        }
                        if (pathable) return true;
                    }
                }
                return false;
            }
            return false;
        case NE:
            if (t.exitable[N_4] && t.exitable[E_4]) {
                for (int i = s; i < s; i++) {
                    for (int j = 4-s; j < 4; j++) {
                        if (t.subcells[(i * 4) + j] == 1)
                            return false;
                    }
                }
                return true;
            }
            return false;
        case E:
            if (t.exitable[E_4]) {
                for (int i = 3; i < 16 - (s * 4); i += 4) {
                    if (t.subcells[i] == 0) {
                        bool pathable = true;
                        for (int j = 1; j < s; j++) {
                            if (t.subcells[i + (j * 4)] == 1) {
                                pathable = false;
                                break;
                            }
                        }
                        if (pathable) return true;
                    }
                }
                return false;
            }
            return false;
        case SE:
            if (t.exitable[S_4] && t.exitable[E_4]) {
                for (int i = 4-s; i < 4; i++) {
                    for (int j = 4 - s; j < 4; j++) {
                        if (t.subcells[(i * 4) + j] == 1)
                            return false;
                    }
                }
                return true;
            }
            return false;
        case S:
            if (t.exitable[S_4]) {
                for (int i = 12; i < 16 - s; i++) {
                    if (t.subcells[i] == 0) {
                        bool pathable = true;
                        for (int j = 1; j < s; j++) {
                            if (t.subcells[i+j] == 1) {
                                pathable = false;
                                break;
                            }
                        }
                        if (pathable) return true;
                    }
                }
                return false;
            }
            return false;
        case SW:
            if (t.exitable[N_4] && t.exitable[W_4]) {
                for (int i = 4 - s; i < 4; i++) {
                    for (int j = 0; j < s; j++) {
                        if (t.subcells[(i * 4) + j] == 1)
                            return false;
                    }
                }
                return true;
            }
            return false;
        case W:
            if (t.exitable[W_4]) {
                for (int i = 0; i < 12 - (s*4); i+=4) {
                    if (t.subcells[i] == 0) {
                        bool pathable = true;
                        for (int j = 1; j < s; j++) {
                            if (t.subcells[i+(j*4)] == 1) {
                                pathable = false;
                                break;
                            }
                        }
                        if (pathable) return true;
                    }
                }
                return false;
            }
            return false;
    }
}

int numExits(const TileData& t) {
    int count = 0;
    for (int i = 0; i < 4; i++) {
        if (t.exitable[i])
            count++;
    }
    return count;
}

bool isPathable(const TileData& t, int size) {

}