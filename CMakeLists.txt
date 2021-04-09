cmake_minimum_required(VERSION 3.19)
project(BraillePaint C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(sdl2-gfx REQUIRED)

add_executable(BraillePaint main.c)
target_link_libraries(BraillePaint PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_gfx)
