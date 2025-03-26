cmake_minimum_required(VERSION "3.30")

set(OPENCV_SUBMODULE_DIR "/home/sandesh/workspace/portapamv2/vendor/opencv")
set(OPENCV_BUILD_DIR "${OPENCV_SUBMODULE_DIR}/build")
set(OPENCV_REPO "https://github.com/opencv/opencv.git")

message("Opencv build dir " ${OPENCV_SUBMODULE_DIR})
# Check if vendor/opencv exists. If not, clone it.
if (NOT EXISTS "${OPENCV_SUBMODULE_DIR}")
    message(STATUS "Cloning OpenCV into ${OPENCV_SUBMODULE_DIR}")
    execute_process(
        COMMAND git clone --depth 1 ${OPENCV_REPO} ${OPENCV_SUBMODULE_DIR}
        RESULT_VARIABLE clone_result
    )

    if(NOT clone_result EQUAL 0)
        message(FATAL_ERROR "OpenCV cloning failed with error code: ${clone_result}")
    endif()
endif()

# Configure and build OpenCV.
if (EXISTS "${OPENCV_SUBMODULE_DIR}" AND EXISTS "${OPENCV_SUBMODULE_DIR}/CMakeLists.txt")
    # Configuration is needed to generate the OpenCVConfig.cmake file we need later.
    execute_process(
        COMMAND
            ${CMAKE_COMMAND}
            -S "${OPENCV_SUBMODULE_DIR}"
            -B "${OPENCV_BUILD_DIR}"
            -G "Ninja"
            -DBUILD_SHARED_LIBS=OFF
            -DBUILD_TESTS=OFF
            -DBUILD_PERF_TESTS=OFF
            -DBUILD_EXAMPLES=OFF
            -DBUILD_JAVA=OFF
            -DBUILD_opencv_core=ON
            -DBUILD_opencv_dnn=ON
            -DBUILD_opencv_imgproc=ON
            -DBUILD_opencv_highgui=OFF
            -DBUILD_opencv_imgcodecs=ON
            -DBUILD_opencv_photo=ON
            -DBUILD_opencv_videoio=OFF
            -DBUILD_opencv_features2d=OFF
            -DBUILD_opencv_ml=OFF
            -DBUILD_opencv_python=OFF
            -DBUILD_opencv_gapi=OFF
            -DBUILD_opencv_flann=OFF
            -DBUILD_opencv_js=OFF
            -DBUILD_OPENEXR=OFF
            -DBUILD_opencv_objc_bindings_gen=OFF
            -DBUILD_opencv_python_bindings_gen=OFF
            -DBUILD_opencv_ts=OFF
            -DBUILD_opencv_video=OFF
            -DBUILD_opencv_python_tests=OFF
            -DWITH_CUDA=OFF
            -DWITH_TIFF=OFF
            -DWITH_VA_INTEL=OFF
            -DWITH_WEBP=OFF
            -DWITH_PNG=OFF
            -DWITH_IPP=OFF
            -DWITH_ITT=OFF
            -DWITH_VA=OFF
            -DWITH_V4L=OFF
            -DWITH_GTK=OFF
            -DWITH_OPENEXR=OFF
            -DWITH_OPENCL=OFF
            -DCV_ENABLE_INTRINSICS=OFF
        RESULT_VARIABLE configure_result
    )

    if(NOT configure_result EQUAL 0)
        message(FATAL_ERROR "OpenCV configuration failed with error code: ${configure_result}")
    endif()
	
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build "${OPENCV_BUILD_DIR}" # Assume build with ninja. make is painful.
    )
else()
    message(FATAL_ERROR "OpenCV submodule directory is missing or invalid.")
endif()

set(OpenCV_DIR "${OPENCV_BUILD_DIR}")
find_package(OpenCV REQUIRED)

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBRARIES}")
message(STATUS "OpenCV include directories: ${OpenCV_INCLUDE_DIRS}")

# Build the project.
add_library(${PROJECT_NAME}_recognition STATIC recognition.cpp)
target_link_libraries(${PROJECT_NAME}_recognition PRIVATE ${OpenCV_LIBRARIES})
target_include_directories(${PROJECT_NAME}_recognition PRIVATE ${OpenCV_INCLUDE_DIRECTORIES})
