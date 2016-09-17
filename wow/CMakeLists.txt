cmake_minimum_required(VERSION 3.5)
project(wow)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        src/main.cpp
        )


set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

add_library(wow SHARED ${SOURCE_FILES})
#add_executable(wow ${SOURCE_FILES})
