# 1. 添加 cmake 版本说明
cmake_minimum_required(VERSION 3.1.3)
# 2. 声明工程名称
project(odb-test)
# 3. 检测并生成 ODB 框架代码
  # a. 添加所需的 ODB 映射代码文件名称
set(odb_path ${CMAKE_CURRENT_SOURCE_DIR}/entity)
set(odb_files student.hxx)
  # b. 检测框架代码文件是否已经生成
set(odb_hxx "")
set(odb_cxx "")
set(odb_srcs "")
foreach(odb_file ${odb_files})
  # c. 如果没有生成，则预定义生成指令 -- 用于在构建项目之间先生成框架代码
  string(REPLACE ".hxx" "-odb.hxx" odb_hxx ${odb_file})
  string(REPLACE ".hxx" "-odb.cxx" odb_cxx ${odb_file})
  if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${odb_cxx})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${odb_cxx}
        COMMAND odb 
        ARGS -d mysql --std c++11 --generate-query --generate-schema --profile boost/date-time ${odb_path}/${odb_file}
        DEPENDS ${odb_path}/${odb_file}
        COMMENT "生成ODB框架代码文件: ${CMAKE_CURRENT_BINARY_DIR}/${odb_cxx}"
    )
  endif()
  # d. 将所有生成的框架源码文件名称保存起来其
  list(APPEND odb_srcs ${CMAKE_CURRENT_BINARY_DIR}/${odb_cxx})
endforeach()
# 4. 获取源码目录下的所有源码文件
set(src_files "")
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source src_files)
# 5. 声明目标及依赖
add_executable(main ${src_files} ${odb_srcs})
# 6. 设置头文件默认搜索路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/entity)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
# 7. 设置需要链接的库
target_link_libraries(main -lodb-mysql -lodb -lodb-boost -lgflags)
# 8. 设置安装路径
INSTALL(TARGETS main RUNTIME DESTINATION bin)