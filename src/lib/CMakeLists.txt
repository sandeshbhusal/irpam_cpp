add_library(
    ${PROJECT_NAME}
    SHARED
    irpam.cpp
)

message("JSON Libs: " ${json_INCLUDE_DIRS})
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    json_INCLUDE_DIRECTORIES
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    ${PROJECT_NAME}_capture
    ${PROJECT_NAME}_recognition
    nlohmann_json
    spdlog::spdlog
)