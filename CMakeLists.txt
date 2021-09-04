cmake_minimum_required(VERSION 3.10)
project(UVRE LANGUAGES C CXX VERSION 0.1.0)

set(UVRE_BACKEND "OpenGL" CACHE STRING "UVRE backend API")
set(UVRE_BUILD_STATIC ON CACHE BOOL "Build static library")

string(TOLOWER "${UVRE_BACKEND}" UVRE_BACKEND_LWR)
if("${UVRE_BACKEND_LWR}" STREQUAL "opengl")
    message("-- UVRE backend API: OpenGL 4.6")
    set(UVRE_OPENGL ON)
    set(UVRE_POSTFIX "gl")
else()
    message(FATAL_ERROR "Unknown backend API: ${UVRE_BACKEND}")
endif()

if(UVRE_STATIC)
    add_library(UVRE STATIC "")
    target_compile_definitions(UVRE PUBLIC UVRE_STATIC)
else()
    add_library(UVRE SHARED "")
    target_compile_definitions(UVRE PRIVATE UVRE_SHARED)
endif()

target_include_directories(UVRE PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
add_subdirectory("src_${UVRE_POSTFIX}")

# TODO: get rid of this in future
add_subdirectory(test)
