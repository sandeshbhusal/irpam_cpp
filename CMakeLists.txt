cmake_minimum_required(VERSION 3.30)
project(irpam)

set(CMAKE_CXX_STANDARD 20)
enable_testing()

add_subdirectory(src/capture)
add_subdirectory(src/recognition)
add_subdirectory(tests)

