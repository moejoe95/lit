cmake_minimum_required(VERSION 3.18)

# set the project name and version
project(lit VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# lit
include_directories(${PROJECT_SOURCE_DIR}/src)
add_executable(lit src/lit.cpp src/commit.cpp src/exec.cpp src/file_utils.cpp src/branch.cpp src/commit_graph.cpp src/checkout.cpp)
