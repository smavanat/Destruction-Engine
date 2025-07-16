#include "../GridData.h"
#include "iostream"
//#include "../GridSystem.h"

//The main grid
GridData mainGrid;
//We are just going to assume that the tiles are going to correspond to marching squares to keep things simple
TileData MS_00;
TileData MS_01;
TileData MS_02;
TileData MS_03;
TileData MS_04;
TileData MS_05;
TileData MS_06;
TileData MS_07;
TileData MS_08;
TileData MS_09;
TileData MS_10;
TileData MS_11;
TileData MS_12;
TileData MS_13;
TileData MS_14;
TileData MS_15;
bool init() {
    //The main grid we are going to use for testing:
    mainGrid.subWidth = 4;
    mainGrid.tileWidth = 32;
    mainGrid.tileHeight = 32;
    mainGrid.gridWidth = 4;
    mainGrid.gridHeight = 4;
    mainGrid.origin = Vector2(0,0);
    mainGrid.tiles = std::vector<TileData>();

    MS_00.status = 0;
    MS_00.type = 0;
    MS_00.subcells = new int[16]{0,0,0,0,
                    0,0,0,0,
                    0,0,0,0,
                    0,0,0,0};
    MS_00.exitable = {true, true, true, true};

    MS_01.status = 2;
    MS_01.type = 0;
    MS_01.subcells = new int[16]{0,0,0,0,
                    0,0,0,0,
                    1,0,0,0,
                    1,1,0,0};
    MS_01.exitable = {true, true, true, true};

    MS_02.status = 2;
    MS_02.type = 0;
    MS_02.subcells = new int[16]{0,0,0,0,
                    0,0,0,0,
                    0,0,0,1,
                    0,0,1,1};
    MS_02.exitable = {true, true, true, true};

    MS_03.status = 2;
    MS_03.type = 0;
    MS_03.subcells = new int[16]{0,0,0,0,
                    0,0,0,0,
                    1,1,1,1,
                    1,1,1,1};
    MS_03.exitable = {true, true, false, true};
    MS_04.status = 2;
    MS_04.type = 0;
    MS_04.subcells = new int[16]{0,0,1,1,
                    0,0,0,1,
                    0,0,0,0,
                    0,0,0,0};
    MS_04.exitable = {true, true, true, true};
    MS_05.status = 2;
    MS_05.type = 0;
    MS_05.subcells = new int[16]{1,1,0,0,
                    1,0,0,0,
                    0,0,0,1,
                    0,0,1,1};
    MS_05.exitable = {true, true, true, true};
    MS_06.status = 2;
    MS_06.type = 0;
    MS_06.subcells = new int[16]{0,0,1,1,
                    0,0,1,1,
                    0,0,1,1,
                    0,0,1,1};
    MS_06.exitable = {true, false, true, true};
    MS_07.status = 2;
    MS_07.type = 0;
    MS_07.subcells = new int[16]{0,0,1,1,
                    0,1,1,1,
                    1,1,1,1,
                    1,1,1,1};
    MS_07.exitable = {true, true, false, false};
    MS_08.status = 2;
    MS_08.type = 0;
    MS_08.subcells = new int[16]{1,1,0,0,
                    1,0,0,0,
                    0,0,0,0,
                    0,0,0,0};
    MS_08.exitable = {true, true, true, true};
    MS_09.status = 2;
    MS_09.type = 0;
    MS_09.subcells = new int[16]{1,1,0,0,
                    1,1,0,0,
                    1,1,0,0,
                    1,1,0,0};
    MS_09.exitable = {true, true, true, false};
    MS_10.status = 2;
    MS_10.type = 0;
    MS_10.subcells = new int[16]{0,0,1,1,
                    0,0,0,1,
                    1,0,0,0,
                    1,1,0,0};
    MS_10.exitable = {true, true, true, true};
    MS_11.status = 2;
    MS_11.type = 0;
    MS_11.subcells = new int[16]{1,1,0,0,
                    1,1,1,0,
                    1,1,1,1,
                    1,1,1,1};
    MS_11.exitable = {true, true, false, false};
    MS_12.status = 2;
    MS_12.type = 0;
    MS_12.subcells = new int[16]{1,1,1,1,
                    1,1,1,1,
                    0,0,0,0,
                    0,0,0,0};
    MS_12.exitable = {false, true, true, true};
    MS_13.status = 2;
    MS_13.type = 0;
    MS_13.subcells = new int[16]{1,1,1,1,
                    1,1,1,1,
                    1,1,1,0,
                    1,1,0,0};
    MS_13.exitable = {false, true, true, false};
    MS_14.status = 2;
    MS_14.type = 0;
    MS_14.subcells = new int[16]{1,1,1,1,
                    1,1,1,1,
                    0,1,1,1,
                    0,0,1,1};
    MS_14.exitable = {false, false, true, true};
    MS_15.status = 2;
    MS_15.type = 0;
    MS_15.subcells = new int[16]{1,1,1,1,
                    1,1,1,1,
                    1,1,1,1,
                    1,1,1,1};
    MS_15.exitable = {false, false, false, false};
    return true;
}

void free() {
    free(MS_00.subcells);
    free(MS_01.subcells);
    free(MS_02.subcells);
    free(MS_03.subcells);
    free(MS_04.subcells);
    free(MS_05.subcells);
    free(MS_06.subcells);
    free(MS_07.subcells);
    free(MS_08.subcells);
    free(MS_09.subcells);
    free(MS_10.subcells);
    free(MS_11.subcells);
    free(MS_12.subcells);
    free(MS_13.subcells);
    free(MS_14.subcells);
    free(MS_15.subcells);
}

bool isValidPosTestPos() {
    init();
    return isValidPos(MS_00.subcells, mainGrid.subWidth, 0, 0, 2) == true && 
            isValidPos(MS_05.subcells, mainGrid.subWidth, 1, 1, 2) == true;
}

bool isValidPosTestNeg() {
    init();
    return isValidPos(MS_15.subcells, mainGrid.subWidth, 0,0, 2) == false &&
            isValidPos(MS_07.subcells, mainGrid.subWidth, 0, 0, 2) == false;
}

int main(int argc, char* args[]) {
    printf("isValidPosTestNeg: %c\n", isValidPosTestPos() ? 'P' : 'F' );
    printf("isValidPosTestNeg: %c\n", isValidPosTestNeg() ? 'P' : 'F' );
    free();
    return 0;
}