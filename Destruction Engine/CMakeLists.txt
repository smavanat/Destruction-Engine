cmake_minimum_required(VERSION 3.10)
project(Tutorial LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Collect all .cpp files in current directory
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Create the executable target
add_executable(my_app ${SOURCES})

# Add include directories (external libraries)
target_include_directories(my_app
    PRIVATE
        "${CMAKE_SOURCE_DIR}/externals/Include"
)

# Link the SDL and Box2D libraries
#Set platform-specific libraries:
if(WIN32) 
    if(MSVC)
        #Set .lib files on msvc
        target_link_libraries(my_app,
            PRIVATE 
            "${CMAKE_SOURCE_DIR}/externals/libs/SDL3.lib"
            "${CMAKE_SOURCE_DIR}/externals/libs/SDL3_image.lib"
            "${CMAKE_SOURCE_DIR}/externals/libs/box2dd.lib"
            user32 gdi32 winmm imm32 ole32 oleaut32 shell32 uuid version
            )
    else()
        target_link_libraries(my_app
            PRIVATE
                "${CMAKE_SOURCE_DIR}/externals/libs/libSDL3.dll.a"
                "${CMAKE_SOURCE_DIR}/externals/libs/libSDL3_image.dll.a"
                "${CMAKE_SOURCE_DIR}/externals/libs/libbox2dd.a"
                mingw32 user32 gdi32 winmm imm32 ole32 oleaut32 shell32 uuid version
        )
    endif()
endif()

# Optional: Copy SDL2.dll to output directory on build (Windows only)
add_custom_command(TARGET my_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/externals/libs/SDL3.dll"
        "${PROJECT_SOURCE_DIR}/externals/libs/SDL3_image.dll"
        $<TARGET_FILE_DIR:my_app>
)

add_custom_command(TARGET my_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        $<TARGET_FILE_DIR:my_app>/assets
)