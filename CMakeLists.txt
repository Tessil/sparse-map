cmake_minimum_required(VERSION 3.1)

project(tsl_sparse_map VERSION 0.5.0)

add_library(sparse_map INTERFACE)
# Use tsl::sparse_map as target, more consistent with other libraries conventions (Boost, Qt, ...)
add_library(tsl::sparse_map ALIAS sparse_map)

target_include_directories(sparse_map INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:include>)

target_sources(sparse_map INTERFACE
               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/sparse_growth_policy.h
                                 ${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/sparse_hash.h
                                 ${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/sparse_map.h
                                 ${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/sparse_set.h>)

if(MSVC)
    target_sources(sparse_map INTERFACE
                   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tsl_sparse_map.natvis>
                   $<INSTALL_INTERFACE:tsl_sparse_map.natvis>)
endif()

# Only compatible with CMake version >= 3.8
if(${CMAKE_VERSION} VERSION_GREATER "3.7")
    target_compile_features(sparse_map INTERFACE cxx_std_11)
endif()




# Installation (only compatible with CMake version >= 3.3)
if(${CMAKE_VERSION} VERSION_GREATER "3.2")
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    ## Install include directory
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/tsl
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})



    ## Create and install tsl_sparse_mapConfig.cmake
    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/tsl_sparse_mapConfig.cmake.in
                                  ${CMAKE_CURRENT_BINARY_DIR}/tsl_sparse_mapConfig.cmake
                                  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tsl_sparse_map)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tsl_sparse_mapConfig.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tsl_sparse_map)

            

    ## Create and install tsl_sparse_mapTargets.cmake
    install(TARGETS sparse_map
            EXPORT tsl_sparse_mapTargets)
            
    install(EXPORT tsl_sparse_mapTargets
            NAMESPACE tsl::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tsl_sparse_map)

            

    ## Create and install tsl_sparse_mapConfigVersion.cmake
    # tsl_sparse_map is header-only and does not depend on the architecture.
    # Remove CMAKE_SIZEOF_VOID_P from tsl_sparse_mapConfigVersion.cmake so that a
    # tsl_sparse_mapConfig.cmake generated for a 64 bit target can be used for 32 bit
    # targets and vice versa.
    set(TSL_SPARSE_MAP_CMAKE_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P})
    unset(CMAKE_SIZEOF_VOID_P)
    write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/tsl_sparse_mapConfigVersion.cmake
                                    COMPATIBILITY SameMajorVersion)
    set(CMAKE_SIZEOF_VOID_P ${TSL_SPARSE_MAP_CMAKE_SIZEOF_VOID_P})

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tsl_sparse_mapConfigVersion.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tsl_sparse_map)
endif()
