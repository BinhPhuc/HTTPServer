cmake_minimum_required(VERSION 3.20)
project(HTTPServer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -pedantic-errors -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion")

find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp")

message(STATUS "Found source files:")
foreach(SOURCE_FILE ${SOURCES})
    message(STATUS "  ${SOURCE_FILE}")
endforeach()

add_executable(server ${SOURCES})

target_compile_definitions(server PRIVATE 
    PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(server PRIVATE 
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build"
)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/build)
