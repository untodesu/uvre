cmake_minimum_required(VERSION 3.10)
project(UVRETest LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
include(FindSDL2)

if(NOT SDL2_FOUND)
    message(FATAL_ERROR "Failed to find SDL2")
endif()

add_executable(test "${CMAKE_CURRENT_LIST_DIR}/test.cpp")
target_link_libraries(test PRIVATE ${UVRE_TARGET} ${SDL2_LIBRARY})
