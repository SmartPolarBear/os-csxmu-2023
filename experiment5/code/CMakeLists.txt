cmake_minimum_required(VERSION 3.21)
project(code)

set(CMAKE_CXX_STANDARD 23)

add_executable(code myRAMFS.cpp Disk.cpp File.cpp)

target_include_directories(code PRIVATE .)