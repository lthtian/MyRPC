
set(SRC_LIST userservice.cc ../user.pb.cc)
add_executable(provider ${SRC_LIST})

# 链接我们搭建的动态库
target_link_libraries(provider mprpc ${PROTOBUF_LIBRARIES})