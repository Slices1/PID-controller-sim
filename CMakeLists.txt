cmake_minimum_required(VERSION 3.5.0)
project(cmake-project-folder)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SourceFiles src/main.cpp)
cmake_policy(SET CMP0074 NEW)

# Find SDL2 and its libraries
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Manually specify SDL2_image and SDL2_ttf paths (adjust if necessary)
set(SDL2_IMAGE_LIB /usr/lib/x86_64-linux-gnu/libSDL2_image.so)
set(SDL2_TTF_LIB /usr/lib/x86_64-linux-gnu/libSDL2_ttf.so)

# Debugging messages
message(STATUS "SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
message(STATUS "SDL2_IMAGE_LIB: ${SDL2_IMAGE_LIB}")
message(STATUS "SDL2_TTF_LIB: ${SDL2_TTF_LIB}")

# Add executable
add_executable(${PROJECT_NAME} ${SourceFiles})

# Link libraries
target_link_libraries(${PROJECT_NAME} SDL2::SDL2 SDL2::SDL2main ${SDL2_IMAGE_LIB} ${SDL2_TTF_LIB})
