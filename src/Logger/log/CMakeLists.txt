# src/Logger/log/CMakeLists.txt
aux_source_directory(. SRC_LOGGER)

add_library(log ${SRC_LOGGER})

target_link_libraries(mymuduo log)