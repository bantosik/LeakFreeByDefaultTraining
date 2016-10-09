cmake_minimum_required(VERSION 3.5)
project(ownership)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES1 main.cpp)
set(SOURCE_FILES2 shared_main.cpp)
add_executable(ownership ${SOURCE_FILES1})
add_executable(shared_main ${SOURCE_FILES2})