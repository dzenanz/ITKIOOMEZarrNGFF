cmake_minimum_required(VERSION 3.16.3)
project(IOOMEZarrNGFF)

set(IOOMEZarrNGFF_LIBRARIES IOOMEZarrNGFF)

if(ITK_SOURCE_DIR)
  message(FATAL_ERROR "${PROJECT_NAME} currently does not support being built as part of ITK_DIR
  It needs to be built of out source, and pointed to a certain ITK build.")
endif()


include(FetchContent)
set(_itk_build_testing ${BUILD_TESTING})
set(BUILD_TESTING OFF)
set(_itk_build_shared ${BUILD_SHARED_LIBS})

find_package(ITK 5.0 REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${ITK_CMAKE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Now specify all the libraries we want to fetch and compile
set(BUILD_CURL_EXE OFF)
FetchContent_Declare(
  CURL
  GIT_REPOSITORY https://github.com/curl/curl
  GIT_TAG        curl-7_84_0
)

set(BUILD_DOC OFF)
set(BUILD_EXAMPLES OFF)
set(BUILD_REGRESS OFF)
set(BUILD_TOOLS OFF)
FetchContent_Declare(
  libzip
  GIT_REPOSITORY https://github.com/nih-at/libzip
  GIT_TAG        v1.9.2
)

set(ZLIB_INCLUDE_DIR ${ITKZLIB_INCLUDE_DIRS})
set(netCDF_ZLIB_INCLUDE_DIR ${ITKZLIB_INCLUDE_DIRS})
set(ZLIB_LIBRARY ${ITKZLIB_LIBRARIES})
set(netcdf_ENABLE_ZSTD OFF)
FetchContent_Declare(
  netCDF
  GIT_REPOSITORY https://github.com/unidata/netcdf-c
  GIT_TAG        v4.9.0
  OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(CURL libzip netCDF)

# Restore build options
set(BUILD_TESTING ${_itk_build_testing})
set(BUILD_SHARED_LIBS ${_itk_build_shared})

find_package(netCDF)

include(ITKModuleExternal)

# find_package(xtensor-zarr CONFIG)

# if(NOT ITK_SOURCE_DIR)
#   find_package(ITK 5.0 REQUIRED)
#   list(APPEND CMAKE_MODULE_PATH ${ITK_CMAKE_DIR})
#   set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#   include(ITKModuleExternal)
# else()
#   set(ITK_DIR ${CMAKE_BINARY_DIR})
#   itk_module_impl()
# endif()
