target_include_directories(${UVRE_TARGET} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/glad/include")
target_sources(${UVRE_TARGET} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/glad/gl.c"
    "${CMAKE_CURRENT_LIST_DIR}/gl_commandlist.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gl_renderdevice.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gl_rmain.cpp")
