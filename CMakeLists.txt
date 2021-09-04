cmake_minimum_required(VERSION 3.10)
project(UVRE LANGUAGES C CXX VERSION 0.1.0)

set(UVRE_BACKEND "OpenGL" CACHE STRING "UVRE backend API")
set(UVRE_BUILD_STATIC ON CACHE BOOL "Build static library")
set(UVRE_BUILD_EXAMPLES ON CACHE BOOL "Build examples")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UVRE_BUILD_STATIC)
    add_library(UVRE STATIC "")
    target_compile_definitions(UVRE PUBLIC UVRE_STATIC)
else()
    add_library(UVRE SHARED "")
    target_compile_definitions(UVRE PRIVATE UVRE_SHARED)
endif()

# Include directories
target_include_directories(UVRE PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

# Backend implementations
message("-- UVRE_BACKEND is ${UVRE_BACKEND}")
string(TOLOWER "${UVRE_BACKEND}" UVRE_BACKEND_LWR)
if("${UVRE_BACKEND_LWR}" STREQUAL "opengl")
    set(UVRE_BACKEND_SHORTNAME "gl")
else()
    message(FATAL_ERROR "!! Invalid UVRE_BACKEND value")
endif()
add_subdirectory("src_${UVRE_BACKEND_SHORTNAME}")

if(UVRE_BUILD_EXAMPLES)
    message("-- Building UVRE examples")
    add_subdirectory(examples)
endif()
