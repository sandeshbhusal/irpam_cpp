cmake_minimum_required(VERSION 3.30)
project(irpam)

set(CMAKE_CXX_STANDARD 20)
enable_testing()

add_library(
    ${PROJECT_NAME}lib
    SHARED
    src/capture/cameramanager.cpp
    src/capture/videodevice.cpp
)

target_include_directories(
    ${PROJECT_NAME}lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/capture/include
)

target_link_libraries(
    ${PROJECT_NAME}lib
    PUBLIC 
        v4l2
)

include(FetchContent)
FetchContent_Declare(
    gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.16.0
)
FetchContent_MakeAvailable(gtest)
include(GoogleTest)

add_executable(
    ${PROJECT_NAME}test
    tests/test_camera.cpp
)

gtest_discover_tests(
    ${PROJECT_NAME}test
)

target_link_libraries(
    ${PROJECT_NAME}test
    PRIVATE
        gtest_main
        ${PROJECT_NAME}lib
)
