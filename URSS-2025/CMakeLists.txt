cmake_minimum_required(VERSION 3.10)
project(Tutorial LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

#Fetch SDL3:
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.18
)

#Fetch SDL3_Image:
FetchContent_Declare (
    SDL3_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG release-3.2.4
)

#Fetch box2D:
FetchContent_Declare (
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v3.1.0
)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SDL3)
FetchContent_MakeAvailable(SDL3_image)
FetchContent_MakeAvailable(box2d)

# Collect all .cpp files in current directory
file(GLOB CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
set(SOURCES ${CPP_SOURCES} ${C_SOURCES})

# Create the executable target
add_executable(urss ${SOURCES})

# Add include directories (external libraries)
target_include_directories(urss
    PRIVATE
        "${CMAKE_SOURCE_DIR}/externals/Include"
)

target_link_libraries(urss PRIVATE SDL3::SDL3 SDL3_image::SDL3_image box2d::box2d)
# target_link_libraries(urss PRIVATE SDL3_image::SDL3_image)

# Have to copy all of the assets to the output directory
add_custom_command(TARGET urss POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        $<TARGET_FILE_DIR:urss>/assets
)
