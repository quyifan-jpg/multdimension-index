cmake_minimum_required(VERSION 3.10)
project(RTree VERSION 1.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# 设置调试/发布模式
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# 添加include目录
include_directories(${CMAKE_SOURCE_DIR})

# 设置RTree库的源文件
set(RTREE_SOURCES
    src/RTree/impl/Point.cpp
    src/RTree/impl/Region.cpp
    src/RTree/impl/Data.cpp
    src/RTree/impl/LeafNode.cpp
    src/RTree/impl/InternalNode.cpp
    src/RTree/impl/SimpleRTree.cpp
)

# 创建RTree静态库
add_library(rtree STATIC ${RTREE_SOURCES})
target_include_directories(rtree PUBLIC ${CMAKE_SOURCE_DIR})

# 添加示例程序
add_executable(rtree_app src/main.cpp)
target_link_libraries(rtree_app PRIVATE rtree)

# 安装规则（可选）
install(TARGETS rtree rtree_app
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(FILES src/RTree/Rtree.h DESTINATION include/RTree)

# 输出配置信息
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}") 