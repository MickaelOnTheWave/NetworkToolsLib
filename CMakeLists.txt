cmake_minimum_required (VERSION 3.2)

project (NetworkToolsLib)

file(GLOB SOURCES "src/*")

add_library(NetworkToolsLib ${SOURCES}) 
target_include_directories(NetworkToolsLib PUBLIC "include")
target_link_libraries (NetworkToolsLib BasicToolsLib)

