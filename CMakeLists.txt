cmake_minimum_required(VERSION 3.30)
project(irpam)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_subdirectory(src/capture)
add_subdirectory(src/recognition)
add_subdirectory(tests)
add_subdirectory(src/lib)

