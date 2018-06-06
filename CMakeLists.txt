cmake_minimum_required(VERSION 3.5.1)
project(PDV_Search)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O3")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(CMAKE_BUILD_TYPE Debug)

add_executable(search main.cpp state.h domains/hanoi.h domains/maze.h domains/sat.h domains/slidingPuzzle.h
        domains/utils.h algorithms/bfs.cpp algorithms/bfs.h algorithms/iddfs.cpp algorithms/iddfs.h)