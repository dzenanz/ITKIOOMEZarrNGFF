set(IOOMEZarrNGFF_SRCS
  itkOMEZarrNGFFImageIO.cxx
  itkOMEZarrNGFFImageIOFactory.cxx
  )

set(TENSORSTORE_USE_SYSTEM_ZLIB ON)
include(FetchContent)
FetchContent_Declare(
  tensorstore
  GIT_REPOSITORY https://github.com/google/tensorstore
  GIT_TAG        f6da8b5696a04cb6f30fab07183756d0d67d5eaa # master
)

FetchContent_MakeAvailable(tensorstore)

itk_module_add_library(IOOMEZarrNGFF ${IOOMEZarrNGFF_SRCS})

target_link_libraries(IOOMEZarrNGFF PRIVATE tensorstore::tensorstore tensorstore::all_drivers)
