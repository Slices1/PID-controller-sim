cmake_minimum_required(VERSION 3.10)
project(PID-Controller)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SourceFiles src/main.cpp)

# 1. Setup PkgConfig
find_package(PkgConfig REQUIRED)

# 2. Find Libraries using PkgConfig (Search for multiple possible names)
pkg_search_module(SDL2 REQUIRED sdl2 SDL2)
pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image sdl2_image)
pkg_search_module(SDL2_TTF REQUIRED SDL2_ttf sdl2_ttf)

# 3. Add Executable
add_executable(${PROJECT_NAME} ${SourceFiles})

# 4. Link Libraries and Includes
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    m # Math library
)