aux_source_directory(. SRC_LIST)
# 生成动态库
add_library(mprpc SHARED ${SRC_LIST})
target_link_libraries(mprpc MyMuduo pthread)