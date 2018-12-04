cmake_minimum_required(VERSION 3.12)
project(kdTree)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

add_executable(kdTree main.cpp src/kdtree.cpp)