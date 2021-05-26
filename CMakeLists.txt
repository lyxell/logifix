cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")

project(logifix)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_CXX_STANDARD 17)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_CLAR OFF)
set(USE_BUNDLED_ZLIB ON)
set(USE_HTTPS OFF)
set(USE_SSH OFF)
set(USE_NTLMCLIENT OFF)
set(REGEX_BACKEND "builtin" CACHE STRING "")
add_subdirectory(vendor/libgit2)

add_subdirectory(sjp)

file(GLOB RULES rules/*.dl)

add_custom_command(
  OUTPUT logifix.cpp
  COMMAND souffle --generate=logifix ${CMAKE_CURRENT_SOURCE_DIR}/program.dl
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/program.dl
  DEPENDS ${RULES}
  VERBATIM)

set_source_files_properties(logifix.cpp PROPERTIES COMPILE_FLAGS -D__EMBEDDED_SOUFFLE__)

add_executable(logifix cli/main.cpp program.cpp logifix.cpp)

target_link_libraries(logifix git2 sjp)
