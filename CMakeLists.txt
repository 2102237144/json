cmake_minimum_required(VERSION 3.26)
project(JSON VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 14)
# 静态库生成的路径
set(LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
# 测试程序生成的路径
set(EXEC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
# 头文件目录
set(HEAD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(src)
add_subdirectory(test)