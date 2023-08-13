# src/timer/CMakeLists.txt
aux_source_directory(. SRC_POLLER)

# 创建 timer 库
add_library(timer ${SRC_POLLER})

# # 添加包含头文件的目录
target_include_directories(timer PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(timer PUBLIC ${CMAKE_SOURCE_DIR}/src/base)


# 将 timer 库链接到 mymuduo 库
target_link_libraries(mymuduo timer)