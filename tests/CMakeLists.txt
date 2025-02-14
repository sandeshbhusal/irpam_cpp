add_executable(
    ${PROJECT_NAME}_tests
    test_camera.cpp
    test_recognition.cpp
)

include(FetchContent)
FetchContent_Declare(
    gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.16.0
)
FetchContent_MakeAvailable(gtest)
include(GoogleTest)

gtest_discover_tests(${PROJECT_NAME}_tests)

target_link_libraries(
    ${PROJECT_NAME}_tests
    PRIVATE
        gtest_main
        ${PROJECT_NAME}_capture
        ${PROJECT_NAME}_recognition
)
