cmake_minimum_required(VERSION 3.5)
project(phobos)

# Add executable
file(GLOB sources src/*)
add_executable(phobos ${sources})

# set_target_properties()
target_compile_options(phobos PUBLIC -std=c++14 -Wall -g)
include_directories("./include/")

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
