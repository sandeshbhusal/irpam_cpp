add_library(
    ${PROJECT_NAME}_recognition
    STATIC
    recognition.cpp
)

include(FetchContent)
FetchContent_Declare (
    libncnn
    GIT_REPOSITORY  https://github.com/Tencent/ncnn
    GIT_TAG         20241226
    GIT_SHALLOW     true
    CMAKE_ARGS      
        -DNCNN_OPENMP=OFF
        -DNCNN_INSTALL_SDK=OFF
        -DNCNN_BUILD_BENCHMARK=OFF
        -DNCNN_VULKAN=OFF
        -DNCNN_BUILD_TESTS=OFF
        -DNCNN_SHARED_LIB=OFF
)

FetchContent_MakeAvailable(libncnn)

target_link_libraries(
    ${PROJECT_NAME}_recognition
    PUBLIC
    ncnn
    ${PROJECT_NAME}_capture
)

target_include_directories(
    ${PROJECT_NAME}_recognition
    PUBLIC
    "include"
)