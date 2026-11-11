cmake_minimum_required(VERSION 3.16)
project(Pong LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 2.0.22 REQUIRED CONFIG)
add_executable(pong main.cpp)
if(TARGET SDL2::SDL2main)
    target_link_libraries(pong PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(pong PRIVATE SDL2::SDL2)
