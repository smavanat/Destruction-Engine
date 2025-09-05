# About  
This is the repository for the code used in the development of my 2025 URSS project *Pathfinding in Dynamic Environments*.  
You can find my current progress logs in the Progress.txt file.

## Building  
This project can be built and run in the following ways:  
### Windows  
```
cd ${DIR-WITH-CODE}  
mkdir build
cd build
cmake -G "${YOUR-CMAKE-BUILD-CONFIG}" ..
cmake --build .
.\urss.exe
```
### Mac and Linux  
```
cd ${DIR-WITH-CODE}  
mkdir build
cd build
cmake ..
make
./urss
```
Building may take a while since the project needs to fetch and build the various libraries to be for your specific system.
## Use  
The app that is built from this project is a demo for the features discussed in my research. The demo consists of three scenarios:  
The first scenario is a grid that is almost entirely covered with impassable tiles except for the start and end tiles. The user can then erase a route through
the tiles that will then then be highlighted by a debug path diagram.  
The second scenario is much like the first, except there is also an agent that follows the outlined path to reach the end.  
The third scenario features no user interaction, since it is a demo of the agent's destruction ability to reach the end tile from the start tile.  
  
If you want to see the parts of the code that are directly relevant to the pathfinding, see GridData.cpp and GridManager.cpp and their respective header files
## Credits  
I am using SDL3: https://github.com/libsdl-org/SDL  
I am using SDL3_Image: https://github.com/libsdl-org/SDL_image  
I am using the box2d physics library: https://github.com/erincatto/box2d  
I am using the polypartition polygon partitioning library: https://github.com/ivanfratric/polypartition  
Catch2 (I am using version 2.13.9, found here:https://github.com/catchorg/Catch2/releases?page=3, but the official code page can be found here: https://github.com/catchorg/Catch2)  
