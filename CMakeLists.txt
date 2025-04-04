cmake_minimum_required(VERSION 3.10)
project(RTree VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set compilation options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Set debug/release mode
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Add include directories
include_directories(${CMAKE_SOURCE_DIR})

# Set source files for RTree library
set(RTREE_SOURCES
    src/RTree/impl/Point.cpp
    src/RTree/impl/Region.cpp
    src/RTree/impl/Data.cpp
    src/RTree/impl/LeafNode.cpp
    src/RTree/impl/InternalNode.cpp
    src/RTree/impl/SimpleRTree.cpp
    src/RTree/impl/LinearSplitStrategy.cpp
    src/RTree/impl/QuadraticSplitStrategy.cpp
    src/RTree/impl/RStarSplitStrategy.cpp
)

# Create RTree static library
add_library(rtree STATIC ${RTREE_SOURCES})
target_include_directories(rtree PUBLIC ${CMAKE_SOURCE_DIR})

# Add example program
add_executable(rtree_app src/main.cpp)
target_link_libraries(rtree_app PRIVATE rtree)

# Installation rules (optional)
install(TARGETS rtree rtree_app
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(FILES src/RTree/Rtree.h DESTINATION include/RTree)

# Output configuration information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}") 