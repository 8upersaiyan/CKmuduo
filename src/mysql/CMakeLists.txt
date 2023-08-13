# src/mysql/CMakeLists.txt
aux_source_directory(. SRC_NET)

# 创建 Net 库
add_library(mysql ${SRC_NET})

# # 添加包含头文件的目录
target_include_directories(mysql PUBLIC ${CMAKE_SOURCE_DIR}/src)
include_directories(/usr/include/mysql)

# 将 Net 库链接到 mymuduo 库
target_link_libraries(mymuduo mysql)