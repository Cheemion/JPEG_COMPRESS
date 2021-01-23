cmake_minimum_required (VERSION 3.8)
project(Decoder)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#${CMAKE_CXX_FLAGS} -g -o2 -Wall
#append the the original with -g -o2 -Wall
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -o2 -Wall")
message(STATUS "The CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")
set(CMAKE_BUILD_TYPE Debug)
add_definitions(-DEXAMPLE_PATH="${EXAMPLE_PATH}")
add_executable(
	Decoder
	Decoder.cpp
)