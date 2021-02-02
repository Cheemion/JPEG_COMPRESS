# CMakeList.txt : CMake project for MPEG, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project(Day1)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


add_definitions(-DEXAMPLE_PATH_BMP="${EXAMPLE_PATH_BMP}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -o2 -Wall")
#set(CMAKE_BUILD_TYPE Debug)

# Add source to this project's executable.
# lib
add_executable(
	Day1
	main.cpp
	src/BMPReader.cpp
)

