cmake_minimum_required(VERSION 3.15)
project(aic_planner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

# add O3 optimization only in release compilation
add_compile_options(
    -Wall
    -Wextra
    $<$<CONFIG:Release>:-O3>
)

add_subdirectory(src)
