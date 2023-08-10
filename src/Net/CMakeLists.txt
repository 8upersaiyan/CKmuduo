# src/Net/CMakeLists.txt
aux_source_directory(. SRC_NET)

# 创建 Net 库
add_library(Net ${SRC_NET})

# # 添加包含头文件的目录
# target_include_directories(Net PUBLIC ${CMAKE_SOURCE_DIR}/src)

# 将 Net 库链接到 mymuduo 库
target_link_libraries(mymuduo Net)
