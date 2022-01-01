cmake_minimum_required(VERSION 3.12)
project(BraillePaint C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(braille_map_gen EXCLUDE_FROM_ALL braille_map_gen.c)

add_library(sfd sfd/sfd.c)
target_include_directories(sfd PUBLIC sfd/)

add_library(bilevel_cvt bilevel_cvt/bilevel_cvt.c)
target_include_directories(bilevel_cvt PUBLIC bilevel_cvt/)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2-gfx CONFIG REQUIRED)
find_package(SDL2-image CONFIG REQUIRED)

add_executable(BraillePaint main.c)
target_link_libraries(BraillePaint PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_gfx SDL2::SDL2_image sfd bilevel_cvt)
