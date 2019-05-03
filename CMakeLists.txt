cmake_minimum_required(VERSION 2.8)
project(dji-dewarp)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -std=c++0x -fopenmp -fPIC")

set (OpenCV_DIR "C:\\dev\\opencv-3.4.5\\build")

find_package( OpenCV REQUIRED )

add_subdirectory(src)

file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
set(files "${SOURCES}" "${HEADERS}")

add_executable(drnmppr-dewarp src/drnmppr-dewarp.cpp)
target_link_libraries(drnmppr-dewarp ${OpenCV_LIBS})