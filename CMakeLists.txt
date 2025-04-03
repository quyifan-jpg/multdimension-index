cmake_minimum_required(VERSION 3.10)
project(MultidimensionIndex VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add extra compilation flags (optional)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

# Find libspatialindex
find_package(libspatialindex QUIET)
if(NOT libspatialindex_FOUND)
    message(STATUS "libspatialindex not found via find_package, trying pkg-config")
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(SPATIALINDEX QUIET libspatialindex)
        if(SPATIALINDEX_FOUND)
            message(STATUS "Found libspatialindex via pkg-config")
        else()
            message(WARNING "libspatialindex not found. Install the library or build without it.")
            message(STATUS "On most systems, it can be installed via package manager:")
            message(STATUS "  - Debian/Ubuntu: apt-get install libspatialindex-dev")
            message(STATUS "  - Fedora/RHEL: dnf install libspatialindex-devel")
            message(STATUS "  - macOS: brew install spatialindex")
            message(STATUS "  - Conda: conda install -c conda-forge libspatialindex")
        endif()
    endif()
endif()

# Collect all source files
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "src/*.h")
file(GLOB R_TREE_SOURCES "src/RTree/*.cpp")
file(GLOB R_TREE_HEADERS "src/RTree/*.h")

# Add executable and link all sources
add_executable(rtree_app ${SOURCES} ${HEADERS} ${R_TREE_SOURCES} ${R_TREE_HEADERS})

# Include header directories
target_include_directories(rtree_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link against libspatialindex if found
if(libspatialindex_FOUND)
    target_link_libraries(rtree_app PRIVATE libspatialindex)
    target_compile_definitions(rtree_app PRIVATE HAVE_LIBSPATIALINDEX)
elseif(SPATIALINDEX_FOUND)
    target_include_directories(rtree_app PRIVATE ${SPATIALINDEX_INCLUDE_DIRS})
    target_link_libraries(rtree_app PRIVATE ${SPATIALINDEX_LIBRARIES})
    target_compile_definitions(rtree_app PRIVATE HAVE_LIBSPATIALINDEX)
endif()

# Installation rules (optional)
install(TARGETS rtree_app DESTINATION bin)

# Add tests (optional, if available)
# enable_testing()
# add_test(NAME rtree_test COMMAND rtree_app test) 