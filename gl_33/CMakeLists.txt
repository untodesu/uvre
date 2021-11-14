target_include_directories(uvre PRIVATE "${CMAKE_CURRENT_LIST_DIR}/glad/include")
target_sources(uvre PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/glad/src/gl33.c"
    "${CMAKE_CURRENT_LIST_DIR}/gl33_commandlist.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gl33_renderdevice.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gl33_rmain.cpp")
