cmake_minimum_required(VERSION 3.14)

project(NetworkToolsTests LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Catch2 REQUIRED)

file(GLOB sources "src/*")

add_subdirectory(libs)

add_executable(NetworkToolsTests ${sources})
target_link_libraries(NetworkToolsTests NetworkToolsLib BasicToolsLib Catch2::Catch2WithMain)

