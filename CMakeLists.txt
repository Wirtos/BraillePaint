cmake_minimum_required(VERSION 3.12)
project(BraillePaint C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(braille_map_gen braille_map_gen.c)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2-gfx CONFIG REQUIRED)
find_package(SDL2-image CONFIG REQUIRED)

add_executable(BraillePaint main.c sfd/sfd.c bilevel_cvt.c)
target_include_directories(BraillePaint PRIVATE sfd ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(BraillePaint PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_gfx SDL2::SDL2_image)
