cmake_minimum_required (VERSION 3.0)
project(mymuduo)

# mymuduo最终编译成so动态库，设置动态库的路径，放在根目录的Lib文件夹下面
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置调试信息 以及启动C++11语言标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")

# 添加搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/src/base
    ${PROJECT_SOURCE_DIR}/src/Net
    ${PROJECT_SOURCE_DIR}/src/Buffer
    ${PROJECT_SOURCE_DIR}/src/Poller
    ${PROJECT_SOURCE_DIR}/src/Logger
    ${PROJECT_SOURCE_DIR}/src/timer
    ${PROJECT_SOURCE_DIR}/src/mysql
    /usr/include/mysql
    )

# 包含这些目录下的.cc文件
aux_source_directory(${PROJECT_SOURCE_DIR}/src/base SRC_BASE)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/Net SRC_NET)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/Buffer SRC_BUFFER)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/Logger SRC_LOG)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/Poller SRC_POLLER)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/timer SRC_TIMER)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/mysql SRC_MYSQL)



# 生成动态库 mymuduo
add_library(mymuduo SHARED 
            ${SRC_BASE}
            ${SRC_NET}
            ${SRC_POLLER}
            ${SRC_LOG}
            ${SRC_BUFFER}
            ${SRC_TIMER}
            ${SRC_MYSQL}
            )

# 链接子目录中的库到 mymuduo
target_link_libraries(mymuduo pthread)