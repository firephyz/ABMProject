cmake_minimum_required(VERSION 3.5)
project(phobos)

# Add executable
file(GLOB sources src/*)
add_executable(phobos ${sources})

# set_target_properties()
target_compile_options(phobos PUBLIC -std=c++14 -Wall -g)
target_link_libraries(phobos dl)
include_directories("./include/")

# Make sure the model library can see the correct symbols given by the runtime
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
