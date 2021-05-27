cmake_minimum_required(VERSION 3.18)

project(logifix)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unknown-pragmas -O2")

set(BUILD_SHARED_LIBS OFF)
set(BUILD_CLAR OFF)
set(USE_BUNDLED_ZLIB ON)
set(USE_HTTPS OFF)
set(USE_SSH OFF)
set(USE_NTLMCLIENT OFF)
set(REGEX_BACKEND "builtin" CACHE STRING "")
add_subdirectory(vendor/libgit2)

add_subdirectory(vendor/souffle)
set_target_properties(souffleprof PROPERTIES EXCLUDE_FROM_ALL 1)

include_directories(vendor/souffle/src/include)

add_subdirectory(vendor/sjp)

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

## TESTS

add_custom_target(test
     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/download_tests.sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests.csv
     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_tests.sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests.csv ${CMAKE_CURRENT_BINARY_DIR}/logifix
)
add_dependencies(test logifix)
