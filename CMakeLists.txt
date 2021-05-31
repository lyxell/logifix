cmake_minimum_required(VERSION 3.18)

project(logifix HOMEPAGE_URL "https://github.com/lyxell/logifix")

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_CXX_STANDARD 17)

configure_file(src/config.h.in ${CMAKE_BINARY_DIR}/config.h)

set(BUILD_SHARED_LIBS OFF)
set(BUILD_CLAR OFF)
set(USE_BUNDLED_ZLIB ON)
set(USE_HTTPS OFF)
set(USE_SSH OFF)
set(USE_NTLMCLIENT OFF)
set(REGEX_BACKEND "builtin" CACHE STRING "")
add_subdirectory(vendor/libgit2)

set(SOUFFLE_USE_CURSES OFF)
add_subdirectory(vendor/souffle)
set_target_properties(souffleprof PROPERTIES EXCLUDE_FROM_ALL 1)

include_directories(vendor/souffle/src/include)

add_subdirectory(vendor/sjp)

file(GLOB RULES src/rules/*.dl)

add_custom_command(
  OUTPUT logifix.cpp
  COMMAND souffle --generate=logifix ${CMAKE_CURRENT_SOURCE_DIR}/src/program.dl
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/program.dl
  DEPENDS ${RULES}
  VERBATIM)

set_source_files_properties(logifix.cpp PROPERTIES COMPILE_FLAGS -D__EMBEDDED_SOUFFLE__)

option(LOGIFIX_BUILD_LIBRARY "Build logifix as a library" OFF)

IF (LOGIFIX_BUILD_LIBRARY)
    add_library(logifix OBJECT src/program.cpp logifix.cpp)
ELSE ()
    add_executable(logifix src/cli.cpp src/program.cpp logifix.cpp)
ENDIF ()

target_link_libraries(logifix git2 sjp)
target_include_directories(logifix PRIVATE ${CMAKE_BINARY_DIR})

## TESTS

add_custom_target(test
     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/download_tests.sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests.csv
     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_tests.sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests.csv ${CMAKE_CURRENT_BINARY_DIR}/logifix)
add_dependencies(test logifix)
