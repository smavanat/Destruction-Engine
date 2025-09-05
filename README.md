# About  
This is the repository for the code used in the development of my 2025 URSS project *Pathfinding in Dynamic Environments*.  
You can find my current progress logs in the Progress.txt file.

## Building  
This project can be built and run in the following ways:  
### Windows:  
```
cd ${DIR-WITH-CODE}  
mkdir build
cd build
cmake -G "${YOUR-CMAKE-BUILD-CONFIG}" ..
cmake --build .
.\urss.exe
```
### Mac and Linux:  
```
cd ${DIR-WITH-CODE}  
mkdir build
cd build
cmake ..
make
./urss
```
Building may take a while since the project needs to fetch and build the various libraries to be for your specific system.
## Credits:  
I am using SDL3: https://github.com/libsdl-org/SDL  
I am using SDL3_Image: https://github.com/libsdl-org/SDL_image  
I am using the box2d physics library: https://github.com/erincatto/box2d  
I am using the polypartition polygon partitioning library: https://github.com/ivanfratric/polypartition  
Catch2 (I am using version 2.13.9, found here:https://github.com/catchorg/Catch2/releases?page=3, but the official code page can be found here: https://github.com/catchorg/Catch2)  
