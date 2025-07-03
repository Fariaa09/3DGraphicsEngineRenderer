cmake_minimum_required(VERSION 3.22)
project(untitled C)

set(CMAKE_C_STANDARD 11)

include_directories(/usr/include/SDL2)

add_executable(untitled main.c)

target_link_libraries(untitled SDL2)
