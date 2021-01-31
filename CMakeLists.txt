# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("JPEG_COMPRESS")
set(EXAMPLE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resource/test.jpg")

# Include sub-projects.
add_subdirectory ("Day1")
add_subdirectory ("Decoder")  