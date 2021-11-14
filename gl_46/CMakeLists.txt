target_include_directories(uvre PRIVATE "${CMAKE_CURRENT_LIST_DIR}/glad/include")
target_sources(uvre PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/glad/src/gl46.c"
    "${CMAKE_CURRENT_LIST_DIR}/gl46_commandlist.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gl46_renderdevice.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gl46_rmain.cpp")
