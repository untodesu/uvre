set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

find_package(glfw3 3.3 REQUIRED)
if(WIN32 AND NOT GLFW_FOUND)
    # I tend to store GLFW there
    add_subdirectory(glfw)
endif()

function(add_example_executable target)
    add_executable(${target} "${CMAKE_CURRENT_LIST_DIR}/${target}.cpp")
    target_compile_definitions(${target} PRIVATE GLFW_INCLUDE_NONE)
    target_link_libraries(${target} PRIVATE glfw uvre)
endfunction()

add_example_executable(base_window)
add_example_executable(triangle)
