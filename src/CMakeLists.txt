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
FetchContent_Declare(Zstd
    URL "https://github.com/facebook/zstd/archive/v1.5.2.zip"
    URL_HASH "SHA256=53f4696f3cec8703f12d3402707a6aaf7eb92d43c90d61e1d32454bda5da7b9c")

FetchContent_MakeAvailable(Zstd)

set(TENSORSTORE_USE_SYSTEM_ZSTD ON)

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
