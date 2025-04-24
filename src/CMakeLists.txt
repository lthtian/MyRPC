aux_source_directory(. SRC_LIST)
#set(SRC_LIST mprpcapplication.cpp mprpcconfig.cpp rpcprovider.cpp rpcheader.pb.cc)
# 生成动态库
add_library(mprpc ${SRC_LIST})
target_link_libraries(mprpc MyMuduo pthread)