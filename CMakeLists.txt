cmake_minimum_required(VERSION 3.24)
project(bst)

set(CMAKE_CXX_STANDARD 17)

add_executable(bst src/main.cpp)

add_subdirectory(tests)
