cmake_minimum_required(VERSION 3.16)
project(tcm)

set(CMAKE_CXX_STANDARD 20)

# Add the "-static-libstdc++" flag to the compiler options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

add_executable(tcm
        matching/tc_basic_type.h
        matching/tc_arg.cpp
        matching/tc_arg.h
        matching/tc_io.cpp
        matching/tc_io.h
        matching/tc_offline_index.cpp
        matching/tc_offline_index.h
        matching/tc_match_order.cpp
        matching/tc_match_order.h
        matching/tc_search.cpp
        matching/tc_search.h
        matching/tc_global_index.cpp
        matching/tc_global_index.h
        matching/tc_misc.cpp
        matching/tc_misc.h
        main.cpp
        matching/tc_g_index.cpp
        matching/tc_g_index.h)
