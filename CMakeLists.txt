cmake_minimum_required(VERSION 3.30)
project(irpam)

set(CMAKE_CXX_STANDARD 20)
enable_testing()

# Define library
add_library(
    ${PROJECT_NAME}lib
    SHARED
    src/capture/camera.cpp
)

# Mark include directories as PUBLIC so that test executables can use camera.hpp
target_include_directories(
    ${PROJECT_NAME}lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/capture/include
)

include(FetchContent)
FetchContent_Declare(
    gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.16.0
)
FetchContent_MakeAvailable(gtest)
include(GoogleTest)

# Define test executable
add_executable(
    ${PROJECT_NAME}test
    tests/camera.cpp
)

# Enable automatic test discovery
gtest_discover_tests(
    ${PROJECT_NAME}test
)

# Link test executable with GoogleTest and irpamlib
target_link_libraries(
    ${PROJECT_NAME}test
    PRIVATE
        gtest_main
        ${PROJECT_NAME}lib
)
