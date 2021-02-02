# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("JPEG_COMPRESS")
set(EXAMPLE_PATH_JPG "${CMAKE_CURRENT_SOURCE_DIR}/resource/lina.jpg")
set(EXAMPLE_PATH_BMP "${CMAKE_CURRENT_SOURCE_DIR}/resource/lina.bmp")


# Include sub-projects.
add_subdirectory ("Decoder")  
add_subdirectory ("Day1")