cmake_minimum_required(VERSION 3.1)

project(tsl_sparse_map)

add_library(tsl_sparse_map INTERFACE)
# Use tsl::sparse_map as target, more consistent with other libraries conventions (Boost, Qt, ...)
add_library(tsl::sparse_map ALIAS tsl_sparse_map)

target_include_directories(tsl_sparse_map INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_sources(tsl_sparse_map INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/sparse_growth_policy.h"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/sparse_hash.h"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/sparse_map.h"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/sparse_set.h")


if(${CMAKE_VERSION} VERSION_GREATER "3.7") 
    # Only available since version 3.8
    target_compile_features(tsl_sparse_map INTERFACE cxx_std_11)
endif()
