#define CATCH_CONFIG_MAIN
#include "../GridData.h"
#include "iostream"
#include "../catch.hpp"
//#include "../GridSystem.h"

//The main grid
std::shared_ptr<GridData> mainGrid;
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
const int size = 2;
const int largeSize = 3;
bool init() {
    //The main grid we are going to use for testing:
    mainGrid = std::make_shared<GridData>();
    mainGrid->subWidth = 4;
    mainGrid->tileWidth = 32;
    mainGrid->tileHeight = 32;
    mainGrid->gridWidth = 4;
    mainGrid->gridHeight = 4;
    mainGrid->origin = Vector2(0,0);
    mainGrid->tiles = std::vector<TileData>();

    MS_00.status = 0;
    MS_00.type = 0;
    MS_00.subcells = {0,0,0,0,
                    0,0,0,0,
                    0,0,0,0,
                    0,0,0,0};
    MS_00.exitable = {true, true, true, true};

    MS_01.status = 2;
    MS_01.type = 0;
    MS_01.subcells = {0,0,0,0,
                    0,0,0,0,
                    1,0,0,0,
                    1,1,0,0};
    MS_01.exitable = {true, true, true, true};

    MS_02.status = 2;
    MS_02.type = 0;
    MS_02.subcells = {0,0,0,0,
                    0,0,0,0,
                    0,0,0,1,
                    0,0,1,1};
    MS_02.exitable = {true, true, true, true};

    MS_03.status = 2;
    MS_03.type = 0;
    MS_03.subcells = {0,0,0,0,
                    0,0,0,0,
                    1,1,1,1,
                    1,1,1,1};
    MS_03.exitable = {true, true, false, true};
    MS_04.status = 2;
    MS_04.type = 0;
    MS_04.subcells = {0,0,1,1,
                    0,0,0,1,
                    0,0,0,0,
                    0,0,0,0};
    MS_04.exitable = {true, true, true, true};
    MS_05.status = 2;
    MS_05.type = 0;
    MS_05.subcells = {1,1,0,0,
                      1,0,0,0,
                      0,0,0,1,
                      0,0,1,1};
    MS_05.exitable = {true, true, true, true};
    MS_06.status = 2;
    MS_06.type = 0;
    MS_06.subcells = {0,0,1,1,
                    0,0,1,1,
                    0,0,1,1,
                    0,0,1,1};
    MS_06.exitable = {true, false, true, true};
    MS_07.status = 2;
    MS_07.type = 0;
    MS_07.subcells = {0,0,1,1,
                    0,1,1,1,
                    1,1,1,1,
                    1,1,1,1};
    MS_07.exitable = {true, true, false, false};
    MS_08.status = 2;
    MS_08.type = 0;
    MS_08.subcells = {1,1,0,0,
                    1,0,0,0,
                    0,0,0,0,
                    0,0,0,0};
    MS_08.exitable = {true, true, true, true};
    MS_09.status = 2;
    MS_09.type = 0;
    MS_09.subcells = {1,1,0,0,
                    1,1,0,0,
                    1,1,0,0,
                    1,1,0,0};
    MS_09.exitable = {true, true, true, false};
    MS_10.status = 2;
    MS_10.type = 0;
    MS_10.subcells = {0,0,1,1,
                    0,0,0,1,
                    1,0,0,0,
                    1,1,0,0};
    MS_10.exitable = {true, true, true, true};
    MS_11.status = 2;
    MS_11.type = 0;
    MS_11.subcells = {1,1,0,0,
                    1,1,1,0,
                    1,1,1,1,
                    1,1,1,1};
    MS_11.exitable = {true, true, false, false};
    MS_12.status = 2;
    MS_12.type = 0;
    MS_12.subcells = {1,1,1,1,
                    1,1,1,1,
                    0,0,0,0,
                    0,0,0,0};
    MS_12.exitable = {false, true, true, true};
    MS_13.status = 2;
    MS_13.type = 0;
    MS_13.subcells = {1,1,1,1,
                    1,1,1,1,
                    1,1,1,0,
                    1,1,0,0};
    MS_13.exitable = {false, true, true, false};
    MS_14.status = 2;
    MS_14.type = 0;
    MS_14.subcells = {1,1,1,1,
                    1,1,1,1,
                    0,1,1,1,
                    0,0,1,1};
    MS_14.exitable = {false, false, true, true};
    MS_15.status = 2;
    MS_15.type = 0;
    MS_15.subcells ={1,1,1,1,
                    1,1,1,1,
                    1,1,1,1,
                    1,1,1,1};
    MS_15.exitable = {false, false, false, false};
    return true;
}

TEST_CASE("isValidPos works", "[pathfinding]") {
    init();
    REQUIRE(isValidPos(MS_00.subcells, mainGrid->subWidth, 0, 0, size) == true);
    REQUIRE(isValidPos(MS_05.subcells, mainGrid->subWidth, 1, 1, size) == true);
    REQUIRE(isValidPos(MS_15.subcells, mainGrid->subWidth, 0, 0, size) == false);
    REQUIRE(isValidPos(MS_07.subcells, mainGrid->subWidth, 0, 0, size) == false);
}

TEST_CASE("preprocessValidPositions works", "[pathfinding]") {
    init();
    REQUIRE(preprocessValidPositions(MS_05.subcells, mainGrid->subWidth, 2) == std::vector<bool>{false, false, true, false, 
                                                                                                false, true, false, false,
                                                                                                true, false, false, false,
                                                                                                false, false, false, false});
}

//Need to see if need to change touches edge because does not take origin touching edge as valid.
TEST_CASE("touchesEdge works", "[pathfinding]") {
    init();
    REQUIRE(touchesEdge(0, 0, size, mainGrid->subWidth) == true); //NW corner
    REQUIRE(touchesEdge(1, 0, size, mainGrid->subWidth) == true); //N side
    REQUIRE(touchesEdge(2, 0, size, mainGrid->subWidth) == true); //NE corner
    REQUIRE(touchesEdge(2, 1, size, mainGrid->subWidth) == true); //E side
    REQUIRE(touchesEdge(2, 2, size, mainGrid->subWidth) == true); //SE corner
    REQUIRE(touchesEdge(1, 2, size, mainGrid->subWidth) == true); //S side
    REQUIRE(touchesEdge(0, 2, size, mainGrid->subWidth) == true); //SW corner
    REQUIRE(touchesEdge(0, 2, size, mainGrid->subWidth) == true); //W side
}

TEST_CASE("checkEdge works", "[pathfinding]") {
    init();
    //Going to check each side in pairs.
    //Each pair will have a set of coordinates that return true and a set of coordinates
    //that return false.

    //NW
    REQUIRE(checkEdge(0,0, size, mainGrid->subWidth, NW) == true);
    REQUIRE(checkEdge(0,1, size, mainGrid->subWidth, NW) == false);
    //NE
    REQUIRE(checkEdge(2,0, size, mainGrid->subWidth, NE) == true);
    REQUIRE(checkEdge(0,0, size, mainGrid->subWidth, NE) == false);
    //SE
    REQUIRE(checkEdge(2,2, size, mainGrid->subWidth, SE) == true);
    REQUIRE(checkEdge(0,0, size, mainGrid->subWidth, SE) == false);
    //SW
    REQUIRE(checkEdge(0,2, size, mainGrid->subWidth, SW) == true);
    REQUIRE(checkEdge(0,0, size, mainGrid->subWidth, SW) == false);
    //N
    REQUIRE(checkEdge(1,0, size, mainGrid->subWidth, N) == true);
    REQUIRE(checkEdge(0,1, size, mainGrid->subWidth, N) == false);
    //E
    REQUIRE(checkEdge(2,1, size, mainGrid->subWidth, E) == true);
    REQUIRE(checkEdge(0,0, size, mainGrid->subWidth, E) == false);
    //S
    REQUIRE(checkEdge(1,2, size, mainGrid->subWidth, S) == true);
    REQUIRE(checkEdge(0,0, size, mainGrid->subWidth, S) == false);
    //W
    REQUIRE(checkEdge(0,1, size, mainGrid->subWidth, W) == true);
    REQUIRE(checkEdge(1,0, size, mainGrid->subWidth, W) == false);
}

TEST_CASE("pathExists works", "[pathfinding]") {
    init();
    //Positive test:
    //Creating the preprocessed array of positions to pass to the function
    std::vector<bool> pArr = preprocessValidPositions(MS_10.subcells, mainGrid->subWidth, size);
    REQUIRE(pathExists(0, 0, size, mainGrid->subWidth, pArr, NW) == true);
    //Negative test:
    pArr = preprocessValidPositions(MS_07.subcells, mainGrid->subWidth, size);
    REQUIRE(pathExists(0, 0, size, mainGrid->subWidth, pArr, NW) == false);
}

TEST_CASE("pathExistsTo works", "[pathfinding]") {
    init();
    //Positive test:
    //Creating the preprocessed array of positions to pass to the function
    std::vector<bool> pArr = preprocessValidPositions(MS_10.subcells, mainGrid->subWidth, size);
    REQUIRE(pathExistsTo(0, 0, mainGrid->subWidth - 1, mainGrid->subWidth - 1, size, mainGrid->subWidth, pArr) == true);
    //Negative test:
    pArr = preprocessValidPositions(MS_07.subcells, mainGrid->subWidth, size);
    REQUIRE(pathExistsTo(0, 0, mainGrid->subWidth - 1, mainGrid->subWidth - 1, size, mainGrid->subWidth, pArr) == false);
}

TEST_CASE("getStartPos works)", "[pathfinding]") {
    init();
    //Going to check each startPosition in pairs, I will just use the empty tile (MS_00) and the filled tile (MS_15) to 
    //use for the positive and negative test
    
    //NW:
    REQUIRE(getStartPos(MS_00.subcells, mainGrid->subWidth, mainGrid->subWidth, size, NW) == std::make_pair(0, 0));
    REQUIRE(getStartPos(MS_15.subcells, mainGrid->subWidth, mainGrid->subWidth,size, NW) == std::make_pair(-1, -1));

    //NE:
    REQUIRE(getStartPos(MS_00.subcells, mainGrid->subWidth, mainGrid->subWidth, size, NE) == std::make_pair(mainGrid->subWidth-size, 0));
    REQUIRE(getStartPos(MS_15.subcells, mainGrid->subWidth, mainGrid->subWidth, size, NE) == std::make_pair(-1, -1));

    //N:
    REQUIRE(getStartPos(MS_00.subcells, mainGrid->subWidth, mainGrid->subWidth, size, N) == std::make_pair(0, 0));
    REQUIRE(getStartPos(MS_15.subcells, mainGrid->subWidth, mainGrid->subWidth, size, N) == std::make_pair(-1, -1));

    //E:
    REQUIRE(getStartPos(MS_00.subcells, mainGrid->subWidth, mainGrid->subWidth, size, E) == std::make_pair(mainGrid->subWidth-size, 0));
    REQUIRE(getStartPos(MS_15.subcells, mainGrid->subWidth, mainGrid->subWidth, size, E) == std::make_pair(-1, -1));

    //SE:
    REQUIRE(getStartPos(MS_00.subcells, mainGrid->subWidth, mainGrid->subWidth, size, SE) == std::make_pair(mainGrid->subWidth-size, mainGrid->subWidth-size));
    REQUIRE(getStartPos(MS_15.subcells, mainGrid->subWidth, mainGrid->subWidth, size, SE) == std::make_pair(-1, -1));

    //S:
    REQUIRE(getStartPos(MS_00.subcells, mainGrid->subWidth, mainGrid->subWidth, size, S) == std::make_pair(0, mainGrid->subWidth-size));
    REQUIRE(getStartPos(MS_15.subcells, mainGrid->subWidth, mainGrid->subWidth, size, S) == std::make_pair(-1, -1));

    //SW:
    REQUIRE(getStartPos(MS_00.subcells, mainGrid->subWidth, mainGrid->subWidth, size, SW) == std::make_pair(0, mainGrid->subWidth-size));
    REQUIRE(getStartPos(MS_15.subcells, mainGrid->subWidth, mainGrid->subWidth, size, SW) == std::make_pair(-1, -1));

    //W:
    REQUIRE(getStartPos(MS_00.subcells, mainGrid->subWidth, mainGrid->subWidth, size, W) == std::make_pair(0, 0));
    REQUIRE(getStartPos(MS_15.subcells, mainGrid->subWidth, mainGrid->subWidth, size, W) == std::make_pair(-1, -1));
}

TEST_CASE("isPathable works", "[pathfinding]") {
    init();

    //Four test cases:
    //Completely passable tile:
    REQUIRE(isPathable(MS_00, N, size, mainGrid->subWidth));
    //Passable partial tile:
    REQUIRE(isPathable(MS_10, N, size, mainGrid->subWidth));
    //Impassable partial tile:
    REQUIRE(!isPathable(MS_07, N, size, mainGrid->subWidth));
    //Completely impassable tile:
    REQUIRE(!isPathable(MS_15, N, size, mainGrid->subWidth));
}

TEST_CASE("combinedTiles works", "[pathfinding]") {
    init();

    //Create the tile ordering vector:
    std::vector<std::vector<int>*> tArr = { &MS_00.subcells, &MS_01.subcells, &MS_02.subcells };
    REQUIRE(combineTiles(tArr, mainGrid->subWidth, 3, 1) == std::vector<int>{0,0,0,0,0,0,0,0,0,0,0,0,
                                                                             0,0,0,0,0,0,0,0,0,0,0,0,
                                                                             0,0,0,0,1,0,0,0,0,0,0,1,
                                                                             0,0,0,0,1,1,0,0,0,0,1,1});
}

TEST_CASE("getCombinedSubcellGrid works", "[pathfinding]") {
    init();

    //Setting up the grid tiles:
    mainGrid->tiles = std::vector<TileData>{MS_02, MS_03, MS_03, MS_01, 
                                            MS_06, MS_00, MS_00, MS_09,
                                            MS_06, MS_00, MS_00, MS_09,
                                            MS_04, MS_12, MS_12, MS_08};
    //Success cases
    REQUIRE(getCombinedSubcellGrid(5, mainGrid, N) == combineTiles(std::vector<std::vector<int>*>{&MS_02.subcells, &MS_03.subcells, &MS_03.subcells}, mainGrid->gridWidth, 3, 1));
    REQUIRE(getCombinedSubcellGrid(4, mainGrid, N) == combineTiles(std::vector<std::vector<int>*>{&MS_02.subcells, &MS_03.subcells}, mainGrid->gridWidth, 2, 1));
    REQUIRE(getCombinedSubcellGrid(7, mainGrid, N) == combineTiles(std::vector<std::vector<int>*>{&MS_03.subcells, &MS_01.subcells}, mainGrid->gridWidth, 2, 1));

    REQUIRE(getCombinedSubcellGrid(5, mainGrid, E) == combineTiles(std::vector<std::vector<int>*>{&MS_03.subcells, &MS_00.subcells, &MS_00.subcells}, mainGrid->gridWidth, 1, 3));
    REQUIRE(getCombinedSubcellGrid(2, mainGrid, E) == combineTiles(std::vector<std::vector<int>*>{&MS_01.subcells, &MS_09.subcells}, mainGrid->gridWidth, 1, 2));
    REQUIRE(getCombinedSubcellGrid(14, mainGrid, E) == combineTiles(std::vector<std::vector<int>*>{&MS_09.subcells, &MS_08.subcells}, mainGrid->gridWidth, 1, 2));

    REQUIRE(getCombinedSubcellGrid(5, mainGrid, S) == combineTiles(std::vector<std::vector<int>*>{&MS_06.subcells, &MS_00.subcells, &MS_00.subcells}, mainGrid->gridWidth, 3, 1));
    REQUIRE(getCombinedSubcellGrid(8, mainGrid, S) == combineTiles(std::vector<std::vector<int>*>{&MS_04.subcells, &MS_12.subcells}, mainGrid->gridWidth, 2, 1));
    REQUIRE(getCombinedSubcellGrid(11, mainGrid, S) == combineTiles(std::vector<std::vector<int>*>{&MS_12.subcells, &MS_08.subcells}, mainGrid->gridWidth, 2, 1));

    REQUIRE(getCombinedSubcellGrid(5, mainGrid, W) == combineTiles(std::vector<std::vector<int>*>{&MS_02.subcells, &MS_06.subcells, &MS_06.subcells}, mainGrid->gridWidth, 1, 3));
    REQUIRE(getCombinedSubcellGrid(1, mainGrid, W) == combineTiles(std::vector<std::vector<int>*>{&MS_02.subcells, &MS_06.subcells}, mainGrid->gridWidth, 1, 2));
    REQUIRE(getCombinedSubcellGrid(13, mainGrid, W) == combineTiles(std::vector<std::vector<int>*>{&MS_06.subcells, &MS_04.subcells}, mainGrid->gridWidth, 1, 2));

    REQUIRE(getCombinedSubcellGrid(5, mainGrid, SE) == combineTiles(std::vector<std::vector<int>*>{&MS_00.subcells, &MS_00.subcells, &MS_00.subcells, &MS_00.subcells}, mainGrid->gridWidth, 2,2));
    REQUIRE(getCombinedSubcellGrid(5, mainGrid, SW) == combineTiles(std::vector<std::vector<int>*>{&MS_06.subcells, &MS_00.subcells, &MS_06.subcells, &MS_00.subcells}, mainGrid->gridWidth, 2,2));
    REQUIRE(getCombinedSubcellGrid(5, mainGrid, NE)== combineTiles(std::vector<std::vector<int>*>{&MS_03.subcells, &MS_03.subcells, &MS_00.subcells, &MS_00.subcells}, mainGrid->gridWidth, 2,2));
    REQUIRE(getCombinedSubcellGrid(5, mainGrid, NW) == combineTiles(std::vector<std::vector<int>*>{&MS_02.subcells, &MS_03.subcells, &MS_06.subcells, &MS_00.subcells}, mainGrid->gridWidth, 2,2));

    //Failure cases:
    REQUIRE(getCombinedSubcellGrid(-1, mainGrid, SE) == std::vector<int>()); //Out of bounds access
    REQUIRE(getCombinedSubcellGrid((mainGrid->subWidth*mainGrid->subWidth) + 1, mainGrid, SE) == std::vector<int>()); //Out of bounds access
    REQUIRE(getCombinedSubcellGrid(1, mainGrid, NE) == std::vector<int>()); //Going NE when on northern edge
    REQUIRE(getCombinedSubcellGrid(7, mainGrid, NE) == std::vector<int>()); //Going NE when on Eastern edge
    REQUIRE(getCombinedSubcellGrid(1, mainGrid, NW) == std::vector<int>()); //Going NW when on northern edge
    REQUIRE(getCombinedSubcellGrid(4, mainGrid, NW) == std::vector<int>()); //Going NW when on western edge
    REQUIRE(getCombinedSubcellGrid(13, mainGrid, SW) == std::vector<int>()); //Going SW when on southern edge
    REQUIRE(getCombinedSubcellGrid(4, mainGrid, SW) == std::vector<int>()); //Going SW when on western edge
    REQUIRE(getCombinedSubcellGrid(13, mainGrid, SE) == std::vector<int>()); //Going SE when on southern edge
    REQUIRE(getCombinedSubcellGrid(7, mainGrid, SE) == std::vector<int>()); //Going SE when on eastern edge
    REQUIRE(getCombinedSubcellGrid(1, mainGrid, N) == std::vector<int>()); //Going N when on northern edge
    REQUIRE(getCombinedSubcellGrid(7, mainGrid, E) == std::vector<int>()); //Going E when on eastern edge
    REQUIRE(getCombinedSubcellGrid(13, mainGrid, S) == std::vector<int>()); //Going S when on southern edge
    REQUIRE(getCombinedSubcellGrid(4, mainGrid, W) == std::vector<int>()); //Going W when on western edge
}

TEST_CASE("getWidth works", "[pathfinding]") {
    init();

    REQUIRE(getWidth(7, mainGrid->gridWidth, N) == 2*mainGrid->gridWidth);
}

TEST_CASE("isPathableWithAdjacent works", "[pathfinding]") {
    init();
    //Setting up the grid tiles:
    mainGrid->tiles = std::vector<TileData>{MS_00, MS_14, MS_09, MS_06,
                                            MS_01, MS_00, MS_00, MS_00,
                                            MS_00, MS_00, MS_00, MS_00,
                                            MS_00, MS_00, MS_00, MS_00};
    // std::vector<int> testGrid = std::vector<int>{1, 1, 0, 0, 0, 0, 1, 1,
    //                                             1, 1, 0, 0, 0, 0, 1, 1, 
    //                                             1, 1, 0, 0, 0, 0, 1, 1, 
    //                                             1, 1, 0, 0, 0, 0, 1, 1};
    // REQUIRE(getCombinedSubcellGrid(7, mainGrid, N) == std::vector<int>{1, 1, 0, 0, 0, 0, 1, 1,
    //                                                                    1, 1, 0, 0, 0, 0, 1, 1, 
    //                                                                    1, 1, 0, 0, 0, 0, 1, 1, 
    //                                                                    1, 1, 0, 0, 0, 0, 1, 1});
    // REQUIRE(isValidPos(testGrid, 8, 2, 0, 3));
    // std::pair<int, int> startPos = getStartPos(testGrid, 8, 4, 3, S);
    // REQUIRE( startPos.first != -1);
    // REQUIRE(startPos.second != -1);
    REQUIRE(isPathableWithAdjacent(7, mainGrid, N, largeSize));
    REQUIRE(isPathableWithAdjacent(9, mainGrid, NW, largeSize));
    REQUIRE(isPathableWithAdjacent(0, mainGrid, SE, largeSize));

}

TEST_CASE("trimCells works", "[pathfinding]"){
    init();

    
}
