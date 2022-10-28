cmake_minimum_required(VERSION 3.24)
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)

project(IOOMEZarrNGFF)

set(IOOMEZarrNGFF_LIBRARIES IOOMEZarrNGFF)

if(ITK_SOURCE_DIR)
  message(FATAL_ERROR "${PROJECT_NAME} currently does not support being built as part of ITK_DIR
  It needs to be built of out source, and pointed to a certain ITK build.")
endif()

if(NOT DEFINED BUILD_TESTING)
  set(BUILD_TESTING TRUE CACHE BOOL "Should ${PROJECT_NAME} tests be built?")
endif()
set(_itk_build_testing ${BUILD_TESTING})
set(BUILD_TESTING OFF CACHE BOOL "tests" FORCE)

set(_itk_build_shared ${BUILD_SHARED_LIBS})

find_package(ITK 5.0 REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${ITK_CMAKE_DIR})
# list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# dump_cmake_variables("ZLIB")



# Restore build options
set(BUILD_TESTING ${_itk_build_testing} CACHE BOOL "tests" FORCE)
set(BUILD_SHARED_LIBS ${_itk_build_shared} CACHE BOOL "DLLs" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include(ITKModuleExternal)


# if(NOT ITK_SOURCE_DIR)
#   find_package(ITK 5.0 REQUIRED)
#   list(APPEND CMAKE_MODULE_PATH ${ITK_CMAKE_DIR})
#   set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#   include(ITKModuleExternal)
# else()
#   set(ITK_DIR ${CMAKE_BINARY_DIR})
#   itk_module_impl()
# endif()
