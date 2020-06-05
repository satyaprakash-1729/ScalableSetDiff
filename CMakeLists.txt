cmake_minimum_required(VERSION 3.15)
project(IBFSetDiff)

set(CMAKE_CXX_STANDARD 14)

add_definitions(-DALPHA=${ALPHA})
add_definitions(-DBETA=${BETA})
add_definitions(-DNUM_HASHES=${NUM_HASHES})
add_executable(IBFSetDiff src/main.cpp src/util.cpp src/util.h)
add_executable(TEST src/p2p_application/main.cpp src/p2p_application/util.cpp src/p2p_application/util.h)
add_executable(CLIENT src/p2p_application/client.cpp src/p2p_application/hosts.h src/p2p_application/IBFSetDiff.h src/p2p_application/util.cpp src/p2p_application/util.h)
add_executable(SERVER src/p2p_application/server.cpp src/p2p_application/hosts.h src/p2p_application/IBFSetDiff.h src/p2p_application/util.cpp src/p2p_application/util.h)
