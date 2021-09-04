cmake_minimum_required(VERSION 3.10)
project(UVRE LANGUAGES C CXX VERSION 0.1.0)

set(UVRE_BACKEND "OpenGL" CACHE STRING "UVRE backend API")
set(UVRE_NOPOSTFIX OFF CACHE BOOL "Keep the target name clean")
set(UVRE_STATIC ON CACHE BOOL "Build static library")

string(TOLOWER "${UVRE_BACKEND}" UVRE_BACKEND)
if("${UVRE_BACKEND}" STREQUAL "opengl")
    message("-- UVRE backend API: OpenGL 4.6")
    set(UVRE_OPENGL ON)
    set(UVRE_POSTFIX "gl")
else()
    message(FATAL_ERROR "Unknown backend API: ${UVRE_BACKEND}")
endif()

if(UVRE_NOPOSTFIX)
    set(UVRE_TARGET "uvre")
else()
    set(UVRE_TARGET "uvre_${UVRE_POSTFIX}")
endif()

if(UVRE_STATIC)
    add_library(${UVRE_TARGET} STATIC "")
    target_compile_definitions(${UVRE_TARGET} PUBLIC UVRE_STATIC)
else()
    add_library(${URVE_TARGET} SHARED "")
    target_compile_definitions(${URVE_TARGET} PRIVATE URVE_SHARED)
endif()

target_include_directories(${UVRE_TARGET} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

if(UVRE_OPENGL)
    add_subdirectory(src_gl)
endif()

add_subdirectory(test)
