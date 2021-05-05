cmake_minimum_required(VERSION 3.10)
project(BraillePaint C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(sdl2-gfx REQUIRED)
find_package(sdl2-image REQUIRED)

add_executable(BraillePaint main.c sfd/sfd.c bilevel_cvt.c)
target_include_directories(BraillePaint PRIVATE sfd ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(BraillePaint PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_gfx SDL2::SDL2_image)
add_executable(braille_map_gen braille_map_gen.c)
