add_library(
    ${PROJECT_NAME}_recognition
    SHARED
    recognition.cpp
)

target_include_directories(
    ${PROJECT_NAME}_recognition
    PUBLIC
    "include"
)