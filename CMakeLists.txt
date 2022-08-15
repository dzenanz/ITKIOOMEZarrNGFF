cmake_minimum_required(VERSION 3.16.3)
cmake_policy(SET CMP0135 NEW)

project(IOOMEZarrNGFF)

set(IOOMEZarrNGFF_LIBRARIES IOOMEZarrNGFF)

if(ITK_SOURCE_DIR)
  message(FATAL_ERROR "${PROJECT_NAME} currently does not support being built as part of ITK_DIR
  It needs to be built of out source, and pointed to a certain ITK build.")
endif()

function(dump_cmake_variables)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        unset(MATCHED)
        string(REGEX MATCH "CMAKE_" MATCHED ${_variableName})
        if (MATCHED) # skip all of CMake's variables - there is a lot of them
            continue()
        endif()

        if (ARGV0)
            unset(MATCHED)
            string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
            if (NOT MATCHED)
                continue()
            endif()
        endif()
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endfunction()

# message("before everything")
# dump_cmake_variables()

include(FetchContent)
set(_itk_build_testing ${BUILD_TESTING})
set(BUILD_TESTING OFF)
set(_itk_build_shared ${BUILD_SHARED_LIBS})

find_package(ITK 5.0 REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${ITK_CMAKE_DIR})
# list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# message("after find ITK")
# dump_cmake_variables()

# Now specify all the libraries we want to fetch and compile
# set(BUILD_CURL_EXE OFF)
# FetchContent_Declare(
#   CURL
#   GIT_REPOSITORY https://github.com/curl/curl
#   GIT_TAG        curl-7_84_0
# )

# set(BUILD_DOC OFF)
# set(BUILD_EXAMPLES OFF)
# set(BUILD_REGRESS OFF)
# set(BUILD_TOOLS OFF)
set(netcdf_ENABLE_ZSTD OFF)
set(ENABLE_ZSTD OFF)
# FetchContent_Declare(
#   libzip
#   GIT_REPOSITORY https://github.com/nih-at/libzip
#   GIT_TAG        v1.9.2
# )

set(ZLIB_INCLUDE_DIR ${ITKZLIB_INCLUDE_DIRS})
set(netCDF_ZLIB_INCLUDE_DIR ${ITKZLIB_INCLUDE_DIRS})
set(ZLIB_LIBRARY ${ITKZLIB_LIBRARIES})

# message("before FetchContent_Declare")
# dump_cmake_variables("ITK_")

# uncommenting this causes duplicate project error
# FetchContent_Declare(
#   hdf5
#   GIT_REPOSITORY https://github.com/HDFGroup/hdf5
#   GIT_TAG        hdf5-1_12_2
# )

# get_filename_component(ITK_SOURCE_DIR ${ITK_USE_FILE} DIRECTORY)
# get_filename_component(ITK_SOURCE_DIR "${ITK_SOURCE_DIR}/../" ABSOLUTE)
# message("ITK_SOURCE_DIR: ${ITK_SOURCE_DIR}")
# set(BUILD_GENERATOR ${CMAKE_GENERATOR})
# set(HDF5_ROOT "${ITK_SOURCE_DIR}/Modules/ThirdParty/HDF5/src/itkhdf5/config/cmake/scripts")
# set(HDF5_ROOT "${ITK_DIR}/Modules/ThirdParty/HDF5/src/itkhdf5")
# message("HDF5_ROOT: ${HDF5_ROOT}")

# set(HDF5_NO_FIND_PACKAGE_CONFIG_FILE TRUE)
# find_package(hdf5 COMPONENTS C HL CONFIG)

# satisfy netCDF's need for HDF5 definitions
set(HDF5_INCLUDE_DIR ${ITKHDF5_INCLUDE_DIRS}) # this needs to be one directory, containing H5pubconf.h and hdf5.h
message("ITKHDF5_INCLUDE_DIRS: ${ITKHDF5_INCLUDE_DIRS}")
# set(HDF5_INCLUDE_DIR "${ITK_SOURCE_DIR}/Modules/ThirdParty/HDF5/src/itkhdf5/src")
# message("HDF5_INCLUDE_DIR: ${HDF5_INCLUDE_DIR}")
set(HDF5_VERSION "1.12.1")
# set(HDF5_C_LIBRARY ${ITKHDF5_LIBRARIES})
# set(HDF5_HL_LIBRARY ${ITKHDF5_LIBRARIES})
set(HDF5_LIBRARIES ${ITKHDF5_LIBRARIES})

# message("after find HDF5")
# dump_cmake_variables("HDF5")

FetchContent_Declare(
  netCDF
  GIT_REPOSITORY https://github.com/dzenanz/netcdf-c # we need a fork, otherwise it is to hard to provide HDF5 to netCDF
  GIT_TAG        itkSpecific # main # v4.9.0
)

# message("HDF5 after FetchContent_Declare")
# dump_cmake_variables("HDF5")

# FetchContent_MakeAvailable(CURL libzip netCDF)
FetchContent_MakeAvailable(netCDF)

# message("after FetchContent_MakeAvailable")
# dump_cmake_variables()

# Restore build options
set(BUILD_TESTING ${_itk_build_testing})
set(BUILD_SHARED_LIBS ${_itk_build_shared})

# find_package(netCDF)

list(APPEND IOOMEZarrNGFF_LIBRARIES netCDF::netcdf)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
dump_cmake_variables()
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
