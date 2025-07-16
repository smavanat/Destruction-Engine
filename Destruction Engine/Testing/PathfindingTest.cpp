#include "GridData.h"
#include "GridSystem.h"

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
    int temp[16] = {0,0,0,0,
                    0,0,0,0,
                    0,0,0,0,
                    0,0,0,0}; 
    MS_00.subcells = temp;
    MS_00.exitable = {true, true, true, true};

    MS_01.status = 2;
    MS_01.type = 0;
    int temp[16] = {0,0,0,0,
                    0,0,0,0,
                    1,0,0,0,
                    1,1,0,0};
    MS_01.subcells = temp;
    MS_01.exitable = {true, true, true, true};

    MS_02.status = 2;
    MS_02.type = 0;
    int temp[16] = {0,0,0,0,
                    0,0,0,0,
                    0,0,0,1,
                    0,0,1,1};
    MS_02.subcells = temp;
    MS_02.exitable = {true, true, true, true};

    MS_03.status = 2;
    MS_03.type = 0;
    int temp[16] = {0,0,0,0,
                    0,0,0,0,
                    1,1,1,1,
                    1,1,1,1};
    MS_03.subcells = temp;
    MS_03.exitable = {true, true, false, true};
    MS_04.status = 2;
    MS_04.type = 0;
    int temp[16] = {0,0,1,1,
                    0,0,0,1,
                    0,0,0,0,
                    0,0,0,0};
    MS_04.subcells = temp;
    MS_04.exitable = {true, true, true, true};
    MS_05.status = 2;
    MS_05.type = 0;
    int temp[16] = {1,1,0,0,
                    1,0,0,0,
                    0,0,0,1,
                    0,0,1,1};
    MS_05.subcells = temp;
    MS_05.exitable = {true, true, true, true};
    MS_06.status = 2;
    MS_06.type = 0;
    int temp[16] = {0,0,1,1,
                    0,0,1,1,
                    0,0,1,1,
                    0,0,1,1};
    MS_06.subcells = temp;
    MS_06.exitable = {true, false, true, true};
    MS_07.status = 2;
    MS_07.type = 0;
    int temp[16] = {0,0,1,1,
                    0,1,1,1,
                    1,1,1,1,
                    1,1,1,1};
    MS_07.subcells = temp;
    MS_07.exitable = {true, true, false, false};
    MS_08.status = 2;
    MS_08.type = 0;
    int temp[16] = {1,1,0,0,
                    1,0,0,0,
                    0,0,0,0,
                    0,0,0,0};
    MS_08.subcells = temp;
    MS_08.exitable = {true, true, true, true};
    MS_09.status = 2;
    MS_09.type = 0;
    int temp[16] = {1,1,0,0,
                    1,1,0,0,
                    1,1,0,0,
                    1,1,0,0};
    MS_09.subcells = temp;
    MS_09.exitable = {true, true, true, false};
    MS_10.status = 2;
    MS_10.type = 0;
    int temp[16] = {0,0,1,1,
                    0,0,0,1,
                    1,0,0,0,
                    1,1,0,0};
    MS_10.subcells = temp;
    MS_10.exitable = {true, true, true, true};
    MS_11.status = 2;
    MS_11.type = 0;
    int temp[16] = {1,1,0,0,
                    1,1,1,0,
                    1,1,1,1,
                    1,1,1,1};
    MS_11.subcells = temp;
    MS_11.exitable = {true, true, false, false};
    MS_12.status = 2;
    MS_12.type = 0;
    int temp[16] = {1,1,1,1,
                    1,1,1,1,
                    0,0,0,0,
                    0,0,0,0};
    MS_12.subcells = temp;
    MS_12.exitable = {false, true, true, true};
    MS_13.status = 2;
    MS_13.type = 0;
    int temp[16] = {1,1,1,1,
                    1,1,1,1,
                    1,1,1,0,
                    1,1,0,0};
    MS_13.subcells = temp;
    MS_13.exitable = {false, true, true, false};
    MS_14.status = 2;
    MS_14.type = 0;
    int temp[16] = {1,1,1,1,
                    1,1,1,1,
                    0,1,1,1,
                    0,0,1,1};
    MS_14.subcells = temp;
    MS_14.exitable = {false, false, true, true};
    MS_15.status = 2;
    MS_15.type = 0;
    int temp[16] = {1,1,1,1,
                    1,1,1,1,
                    1,1,1,1,
                    1,1,1,1};
    MS_01.subcells = temp;
    MS_01.exitable = {false, false, false, false};
}

bool isValidPosTestPos() {
    
}

bool isValidPosTestNeg() {

}