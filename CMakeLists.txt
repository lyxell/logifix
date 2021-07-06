cmake_minimum_required(VERSION 3.18)

project(logifix HOMEPAGE_URL "https://github.com/lyxell/logifix")

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_CXX_STANDARD 17)

### DEPENDENCIES

add_subdirectory(vendor/fmt)

add_subdirectory(vendor/nway)

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

# We need this to be global for project to allow sjp to use the Souffle headers
include_directories(vendor/souffle/src/include)

add_subdirectory(vendor/sjp)

### SOURCE

find_package(Git)

if(GIT_EXECUTABLE)
  # Generate a git-describe version string from Git repository tags
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --match "v*"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
    RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(NOT GIT_DESCRIBE_ERROR_CODE)
    set(LOGIFIX_VERSION ${GIT_DESCRIBE_VERSION})
  endif()
endif()

if(NOT DEFINED LOGIFIX_VERSION)
  set(LOGIFIX_VERSION v0.0.0-unknown)
  message(WARNING "Failed to determine LOGIFIX_VERSION from Git tags. Using default version \"${LOGIFIX_VERSION}\".")
endif()

message(STATUS "Building logifix version ${LOGIFIX_VERSION}")

file(GLOB_RECURSE RULE_IMPLEMENTATION_FILES src/rules/*.dl)
file(GLOB_RECURSE RULE_DATA_FILES src/rules/*.json)

add_custom_command(
  OUTPUT logifix.cpp
  COMMAND souffle --generate=logifix ${CMAKE_CURRENT_SOURCE_DIR}/src/program.dl
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/program.dl
  DEPENDS ${RULE_IMPLEMENTATION_FILES}
  VERBATIM)

add_custom_command(
  OUTPUT rule_data.cpp
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_rule_data.sh rule_data.cpp
  DEPENDS ${RULE_DATA_FILES}
  VERBATIM)

set_source_files_properties(logifix.cpp PROPERTIES COMPILE_FLAGS -D__EMBEDDED_SOUFFLE__)

option(LOGIFIX_BUILD_LIBRARY "Build logifix as a library" OFF)

IF (LOGIFIX_BUILD_LIBRARY)
    add_library(logifix OBJECT src/program.cpp logifix.cpp)
ELSE ()
    add_executable(logifix src/libgit.cpp src/cli.cpp src/tty.cpp src/program.cpp logifix.cpp rule_data.cpp)
ENDIF ()
target_include_directories(logifix PRIVATE ${CMAKE_BINARY_DIR})
target_include_directories(logifix PRIVATE vendor/libgit2/include)

target_include_directories(logifix PRIVATE vendor/fmt/include)

target_link_libraries(logifix git2 sjp nway fmt -static-libgcc -static-libstdc++)

configure_file(src/config.h.in ${CMAKE_BINARY_DIR}/config.h)

### TESTS

add_custom_target(test_local
     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_tests.sh ${CMAKE_CURRENT_BINARY_DIR}/logifix)

add_custom_target(test
     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/real_world/download_tests.sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/real_world/tests.csv
     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/real_world/run_tests.sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/real_world/tests.csv ${CMAKE_CURRENT_BINARY_DIR}/logifix
     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_tests.sh ${CMAKE_CURRENT_BINARY_DIR}/logifix)

add_dependencies(test logifix)
