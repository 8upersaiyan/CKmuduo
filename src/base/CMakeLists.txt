# src/base/CMakeLists.txt
aux_source_directory(. SRC_BASE)

# 创建 base 库
add_library(base ${SRC_BASE})

# # 添加包含头文件的目录
# target_include_directories(base PUBLIC ${CMAKE_SOURCE_DIR}/src)

# 将 base 库链接到 mymuduo 库
target_link_libraries(mymuduo base)
