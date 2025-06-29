cmake_minimum_required(VERSION 3.15)
project(ZLog)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 fmt 库
find_package(fmt REQUIRED)

# 日志库头文件路径
include_directories(${CMAKE_CURRENT_LIST_DIR}/../zlog)

# bench 测试程序
add_executable(bench bench.cc)
target_link_libraries(bench PRIVATE fmt::fmt pthread)
