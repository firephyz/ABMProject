cmake_minimum_required(VERSION 3.5)
project(gamp)
# Set output to be verbose
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Add executable
file(GLOB sources src/*)
add_executable(gamp ${sources})

# set_target_properties()
target_compile_options(gamp PUBLIC -std=c++14 -Wall -g)
target_link_libraries(gamp xml2)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
