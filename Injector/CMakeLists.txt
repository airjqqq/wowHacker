cmake_minimum_required(VERSION 3.5)
project(Injector)

set(SOURCE_FILES
        src/ReflectiveDLLInjection.h
        src/LoadLibraryR.h
        src/LoadLibraryR.cpp
        src/main.cpp
        src/memory.h src/memory.cpp)
add_executable(Injector ${SOURCE_FILES})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

