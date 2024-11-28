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
add_executable(lab4
        lab4/main.cpp
        lab4/BMPUtils.h
)
add_executable(lab5_1
        lab5/main_1.cpp
)
add_executable(lab5_2
        lab5/main_2.cpp
)

find_package(OpenMP REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
add_executable(lab6_1
        lab6/main_1.cpp
)
add_executable(lab6_2
        lab6/main_2.cpp
)
add_executable(lab6_3
        lab6/main_3.cpp
)
if(OpenMP_CXX_FOUND)
    target_link_libraries(lab6_1 PRIVATE OpenMP::OpenMP_CXX)
    target_link_libraries(lab6_2 PRIVATE OpenMP::OpenMP_CXX)
    target_link_libraries(lab6_3 PRIVATE OpenMP::OpenMP_CXX)
endif()