# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
project(Decoder)
# Add source to this project's executable.
# lib
add_executable(
	Decoder
	Decoder.cpp
)
