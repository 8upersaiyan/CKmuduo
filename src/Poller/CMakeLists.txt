# src/Poller/CMakeLists.txt
aux_source_directory(. SRC_POLLER)

# 创建 Poller 库
add_library(Poller ${SRC_POLLER})

# # 添加包含头文件的目录
# target_include_directories(Poller PUBLIC ${CMAKE_SOURCE_DIR}/src)

# 将 Poller 库链接到 mymuduo 库
target_link_libraries(mymuduo Poller)
