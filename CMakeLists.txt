cmake_minimum_required(VERSION 3.22)
project(untitled C)
set(CMAKE_C_STANDARD 11)


# Directories we need to include ----------------
include_directories(/usr/include/SDL2)


# Make the executable ---------------------------
add_executable(untitled main.c)


# Link the target libraries ---------------------
# This is to link sdl2
target_link_libraries(untitled SDL2)
# This is to link libm for the math.h include
target_link_libraries(untitled m)

