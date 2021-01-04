# CMakeList.txt : CMake project for MPEG, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project(Day1)
# Add source to this project's executable.
# lib
add_library(
	Day1
	JpegEncoder.cpp
)

#头文件 header
target_include_directories(Day1 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# TODO: Add tests and install targets if needed.
