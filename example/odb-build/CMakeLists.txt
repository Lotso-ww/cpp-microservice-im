cmake_minimum_required(VERSION 3.1.3)
project(all_test)

# 让 IDE/clangd 能拿到 include 路径和编译选项，消除红线
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/odb-test-2)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})