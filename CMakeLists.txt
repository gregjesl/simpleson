# CMakeList.txt : CMake project for JSON, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project(simpleson VERSION 2.0.0)

set (CMAKE_CXX_STANDARD 98)

enable_testing()
add_subdirectory (test)


# TODO: Add tests and install targets if needed.
