# Usage (Linux):
#   $ cd OpenTombSrc
#   $ mkdir build && cd build
#   $ cmake .. && make

cmake_minimum_required(VERSION 2.8)

project(OpenTomb)

message(WARNING "This CMakeLists.txt does not generate executables, it is only used to process the documentation.")

add_subdirectory(doc)
