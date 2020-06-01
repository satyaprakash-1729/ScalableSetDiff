cmake_minimum_required(VERSION 3.15)
project(IBFSetDiff)

set(CMAKE_CXX_STANDARD 14)

add_definitions(-DALPHA=${ALPHA})
add_definitions(-DBETA=${BETA})
add_definitions(-DNUM_HASHES=${NUM_HASHES})
add_executable(IBFSetDiff src/main.cpp src/util.cpp src/util.h)