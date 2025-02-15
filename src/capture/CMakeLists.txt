add_library(
    ${PROJECT_NAME}_capture
    SHARED
    cameramanager.cpp
    videodevice.cpp
    image.cpp
)

target_include_directories(
    ${PROJECT_NAME}_capture
    PUBLIC
    "include"
)

target_link_libraries(
    ${PROJECT_NAME}_capture
    PRIVATE
        v4l2
        spdlog
)