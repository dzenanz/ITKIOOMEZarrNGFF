set(IOOMEZarrNGFF_SRCS
  itkOMEZarrNGFFImageIO.cxx
  itkOMEZarrNGFFImageIOFactory.cxx
  )


# We use tensorstore backend
set(TENSORSTORE_USE_SYSTEM_ZLIB ON) # Provide ITK's bundled zlib
set(ZLIB_INCLUDE_DIR ${ITKZLIB_INCLUDE_DIRS} CACHE PATH "Path to zlib include" FORCE)
set(ZLIB_LIBRARY ${ITKZLIB_LIBRARIES} CACHE FILEPATH "Path to zlib library" FORCE)
add_library(ZLIB::ZLIB ALIAS ${ITKZLIB_LIBRARIES})

set(_itk_build_shared ${BUILD_SHARED_LIBS})
# we want to compile tensorstore as a static library
# that way, all of it can be included in IOOMEZarrNGFF.dll
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries?" FORCE)

include(FetchContent)
# TensorStore has problems building its bundled zstd, so we build it here
option(ZSTD_BUILD_CONTRIB "BUILD_CONTRIB" OFF)
option(ZSTD_BUILD_PROGRAMS "BUILD_PROGRAMS" OFF)
option(ZSTD_BUILD_SHARED "BUILD_SHARED" OFF)
option(ZSTD_BUILD_STATIC "BUILD_STATIC" ON)
option(ZSTD_BUILD_TESTS "BUILD_TESTS" OFF)
option(ZSTD_BUILD_LEGACY_SUPPORT "BUILD_LEGACY_SUPPORT" OFF)
option(ZSTD_MULTITHREAD_SUPPORT "BUILD_MULTITHREAD_SUPPORT" OFF)
option(ZSTD_BUILD_PROGRAMS_LINK_SHARED "BUILD_PROGRAMS_LINK_SHARED" OFF)
option(ZSTD_BUILD_LZ4 "BUILD_LZ4" OFF)
option(ZSTD_BUILD_LZMA "BUILD_LZMA" OFF)
option(ZSTD_BUILD_ZLIB "BUILD_ZLIB" OFF)
set(zstd_GIT_REPOSITORY "https://github.com/facebook/zstd.git")
# v1.5.2
set(zstd_GIT_TAG c9c7be85f49f45a581ec00c309afda5c62ba9ef2)
FetchContent_Declare(
  zstd_lib
  GIT_REPOSITORY ${zstd_GIT_REPOSITORY}
  GIT_TAG        ${zstd_GIT_TAG}
)
FetchContent_MakeAvailable(zstd_lib)

add_subdirectory("${zstd_lib_SOURCE_DIR}/build/cmake" "${zstd_lib_BINARY_DIR}")

set(zstd_DIR ${zstd_lib_BINARY_DIR})
find_package(zstd CONFIG REQUIRED) # find it now, so tensorstore finds the same zstd later
add_library(Zstd::Zstd ALIAS libzstd_static)
# set(TENSORSTORE_USE_SYSTEM_ZSTD ON)

set(CMAKE_FOLDER TensorStore)
FetchContent_Declare(
  tensorstore
  GIT_REPOSITORY https://github.com/dzenanz/tensorstore # https://github.com/google/tensorstore
  GIT_TAG        10ae2c2d1bd270338a59ce5aa122fb111a96e893
)
FetchContent_MakeAvailable(tensorstore)
unset(CMAKE_FOLDER)
set(BUILD_SHARED_LIBS ${_itk_build_shared} CACHE BOOL "Build shared libraries?" FORCE) # restore original flag
unset(_itk_build_shared)

itk_module_add_library(IOOMEZarrNGFF ${IOOMEZarrNGFF_SRCS})

target_link_libraries(IOOMEZarrNGFF PRIVATE libzstd_static tensorstore::tensorstore tensorstore::all_drivers)
