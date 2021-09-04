target_include_directories(UVRE PRIVATE "${CMAKE_CURRENT_LIST_DIR}/glad/include")
target_sources(UVRE PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/glad/src/gl.c"
    "${CMAKE_CURRENT_LIST_DIR}/gl_commandlist.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gl_renderdevice.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gl_rmain.cpp")
