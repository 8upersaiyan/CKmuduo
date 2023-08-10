# src/Buffer/CMakeLists.txt
aux_source_directory(. SRC_BUFFER)

# 创建 Buffer 库
add_library(Buffer ${SRC_BUFFER})

# # 添加包含头文件的目录
# target_include_directories(Buffer PUBLIC ${CMAKE_SOURCE_DIR}/src)

# 将 Buffer 库链接到 mymuduo 库
target_link_libraries(mymuduo Buffer)
