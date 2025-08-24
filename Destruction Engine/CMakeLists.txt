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

FetchContent_Declare (
    SDL_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG release-3.2.4
)

set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SDL3)
FetchContent_MakeAvailable(SDL_Image)

# Collect all .cpp files in current directory
file(GLOB CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
set(SOURCES ${CPP_SOURCES} ${C_SOURCES})

# Create the executable target
add_executable(my_app ${SOURCES})

# Add include directories (external libraries)
target_include_directories(my_app
    PRIVATE
        "${CMAKE_SOURCE_DIR}/externals/Include"
)

# Link the SDL and Box2D libraries
# Set platform-specific libraries:
# if(WIN32) 
#     if(MSVC)
#         # Set .lib files on msvc
#         target_link_libraries(my_app
#             PRIVATE 
#             "${CMAKE_SOURCE_DIR}/externals/libs/Windows/SDL3.lib"
#             "${CMAKE_SOURCE_DIR}/externals/libs/Windows/SDL3_image.lib"
#             "${CMAKE_SOURCE_DIR}/externals/libs/Windows/box2dd.lib"
#             user32 gdi32 winmm imm32 ole32 oleaut32 shell32 uuid version
#             )
#     else()
#     # Assume other users are using MinGW.
#         target_link_libraries(my_app
#             PRIVATE
#                 "${CMAKE_SOURCE_DIR}/externals/libs/Windows/libSDL3.dll.a"
#                 "${CMAKE_SOURCE_DIR}/externals/libs/Windows/libSDL3_image.dll.a"
#                 "${CMAKE_SOURCE_DIR}/externals/libs/Windows/libbox2dd.a"
#                 mingw32 user32 gdi32 winmm imm32 ole32 oleaut32 shell32 uuid version
#         )
#     endif()
#
#     # Copy .dll to output directory on build (Windows only)
#     add_custom_command(TARGET my_app POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different
#             "${PROJECT_SOURCE_DIR}/externals/libs/SDL3.dll"
#             "${PROJECT_SOURCE_DIR}/externals/libs/SDL3_image.dll"
#             $<TARGET_FILE_DIR:my_app>
#     )
# elseif(UNIX AND NOT APPLE)
#     #Linux specific settings:
#     target_link_libraries(my_app
#         PRIVATE
#         #"${CMAKE_SOURCE_DIR}/externals/libs/Linux/libSDL3.a"
#         #"${CMAKE_SOURCE_DIR}/externals/libs/Linux/libSDL3_image.dll.a"
#         "${PROJECT_SOURCE_DIR}/externals/libs/Linux/libSDL3.so.0.2.18"
#         "${PROJECT_SOURCE_DIR}/externals/libs/Linux/libSDL3_image.so.0.2.4"
#         "${CMAKE_SOURCE_DIR}/externals/libs/Linux/libbox2d.a")
#
#     # Add .so for SDL3_image post build
#     add_custom_command(TARGET my_app POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different
#             "${PROJECT_SOURCE_DIR}/externals/libs/Linux/libSDL3_image.so.0"
#             "${PROJECT_SOURCE_DIR}/externals/libs/Linux/libSDL3.so.0"
#             $<TARGET_FILE_DIR:my_app>
#     )
# endif()

target_link_libraries(my_app PRIVATE SDL3::SDL3 SDL_image::SDL_image)
# target_link_libraries(my_app PRIVATE SDL_image::SDL_image)

# Have to copy all of the assets to the output directory
add_custom_command(TARGET my_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        $<TARGET_FILE_DIR:my_app>/assets
)
