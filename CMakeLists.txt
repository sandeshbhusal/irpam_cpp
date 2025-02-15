cmake_minimum_required(VERSION 3.30)
project(irpam)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_SHALLOW    TRUE
    GIT_TAG        v2.x
    CMAKE_ARGS
        -DSPDLOG_BUILD_PIC=ON
)
FetchContent_MakeAvailable(spdlog)
set(SPDLOG_BUILD_SHARED ON CACHE BOOL "" FORCE)

add_subdirectory(src/capture)
add_subdirectory(src/recognition)
add_subdirectory(tests)
add_subdirectory(src/lib)