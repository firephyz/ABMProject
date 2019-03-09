cmake_minimum_required(VERSION 3.5)
project(gamp)

# Add executable
file(GLOB sources src/*)
add_executable(gamp ${sources})

# set_target_properties()
target_compile_options(gamp PUBLIC -std=c++14 -Wall -g)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
