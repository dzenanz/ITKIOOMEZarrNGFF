cmake_minimum_required(VERSION 3.16.3)
cmake_policy(SET CMP0135 NEW)

project(IOOMEZarrNGFF-Examples)

find_package(ITK 5.0 REQUIRED IOOMEZarrNGFF)

add_executable(SimpleWriteRead SimpleWriteRead.cxx)

target_link_libraries(SimpleWriteRead ${ITK_LIBRARIES})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SimpleWriteRead)
