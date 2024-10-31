cmake_minimum_required(VERSION 3.26)
project(ParallelProgramming)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

add_executable(lab1
        lab1/main.cpp
)

add_executable(lab2
        lab2/main.cpp
        lab2/BMPUtils.h
)
add_executable(lab3
        lab3/main.cpp
)
