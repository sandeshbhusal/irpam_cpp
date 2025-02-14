add_library(
    ${PROJECT_NAME}
    SHARED
    irpam.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    ${PROJECT_NAME}_capture
    ${PROJECT_NAME}_recognition
)