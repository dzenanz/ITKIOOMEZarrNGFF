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

# TensorStore wants GCC as assembler
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER}) # undo this setting from:
# https://github.com/dockcross/dockcross/blob/e913a7f1d632940a4e6fb233fb6b5cdfa8c4502d/manylinux_2_28-x64/Toolchain.cmake#L11

# we want to compile tensorstore as a static library
# that way, all of it can be included in IOOMEZarrNGFF.dll
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries?" FORCE)
set(CMAKE_FOLDER TensorStore)
include(FetchContent)
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

target_link_libraries(IOOMEZarrNGFF PRIVATE tensorstore::tensorstore tensorstore::all_drivers)
