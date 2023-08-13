# src/Logger/CMakeLists.txt
aux_source_directory(. SRC_LOGGER)

# 创建 Logger 库
add_library(Logger ${SRC_LOGGER})

# # 添加包含头文件的目录
target_include_directories(Logger PUBLIC ${CMAKE_SOURCE_DIR}/src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/log)

# 将 Logger 库链接到 mymuduo 库
target_link_libraries(mymuduo Logger)
