cmake_minimum_required (VERSION 3.8)
project(Decoder)
include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(
	Decoder
	Decoder.cpp
)
