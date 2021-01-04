# CMakeList.txt : CMake project for MPEG, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project(JPEG)

# Add source to this project's executable.
add_executable (JPEG "JPEG.cpp")
target_link_libraries(JPEG PRIVATE Day1)

# TODO: Add tests and install targets if needed.
