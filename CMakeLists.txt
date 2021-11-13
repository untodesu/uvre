cmake_minimum_required(VERSION 3.10)
project(uvre LANGUAGES C CXX VERSION 0.1.0)

set(UVRE_IMPL "GL_46" CACHE STRING "UVRE implementation API")
set(UVRE_BUILD_STATIC ON CACHE BOOL "Build static library")
set(UVRE_BUILD_EXAMPLES ON CACHE BOOL "Build examples")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UVRE_BUILD_STATIC)
    add_library(uvre STATIC "")
    target_compile_definitions(uvre PUBLIC UVRE_STATIC)
else()
    add_library(uvre SHARED "")
    target_compile_definitions(uvre PRIVATE UVRE_SHARED)
endif()

# Include directories
target_include_directories(uvre PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

# API implementations
message("-- UVRE_IMPL is ${UVRE_IMPL}")
string(TOLOWER "${UVRE_IMPL}" UVRE_IMPL_LWR)
add_subdirectory("${UVRE_IMPL_LWR}")

if(UVRE_BUILD_EXAMPLES)
    message("-- Building UVRE examples")
    add_subdirectory(examples)
endif()
