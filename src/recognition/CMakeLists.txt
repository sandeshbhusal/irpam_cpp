cmake_minimum_required(VERSION "3.30")

# Build the project.
add_library(
    ${PROJECT_NAME}_recognition
    STATIC
    recognition.cpp
)

target_link_libraries(
    ${PROJECT_NAME}_recognition
    
    PUBLIC 
    ${OpenCV_LIBRARIES}
)

target_include_directories(
    ${PROJECT_NAME}_recognition 

    PUBLIC 
    ${OpenCV_INCLUDE_DIRS}
)
