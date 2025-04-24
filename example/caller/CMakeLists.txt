
set(SRC_LIST calluserservice.cc ../user.pb.cc)
add_executable(consumer ${SRC_LIST})

# 链接我们搭建的动态库
target_link_libraries(consumer mprpc ${PROTOBUF_LIBRARIES})