cmake_minimum_required(VERSION 3.18)

project(logifix HOMEPAGE_URL "https://github.com/lyxell/logifix")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

### DEPENDENCIES

add_subdirectory(vendor/fmt)
add_subdirectory(vendor/nway)
add_subdirectory(vendor/souffle)
# We need this to be global for project to allow sjp to use the Souffle headers
include_directories(vendor/souffle/src/include)
add_subdirectory(vendor/sjp)

### SOURCE

#### Set version
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

#### Find files
file(GLOB_RECURSE RULE_IMPLEMENTATION_FILES src/rules/*.dl)
file(GLOB_RECURSE RULE_DATA_FILES src/rules/*.json)

#### Generate logifix.cpp from Datalog code
add_custom_command(
  OUTPUT logifix.cpp
  COMMAND souffle --generate=logifix ${CMAKE_CURRENT_SOURCE_DIR}/src/program.dl
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/program.dl
  DEPENDS ${RULE_IMPLEMENTATION_FILES}
  VERBATIM)
set_source_files_properties(logifix.cpp PROPERTIES COMPILE_FLAGS -D__EMBEDDED_SOUFFLE__)

#### Generate rule_data.cpp from json files
add_custom_command(
  OUTPUT rule_data.cpp
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_rule_data.sh rule_data.cpp
  DEPENDS ${RULE_DATA_FILES}
  VERBATIM)

#### Generate config.h from config.h.in
configure_file(src/config.h.in ${CMAKE_BINARY_DIR}/config.h)

#### Create executable
add_executable(logifix src/cli.cpp src/tty.cpp src/javadoc.cpp src/logifix.cpp logifix.cpp rule_data.cpp)
target_include_directories(logifix PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(logifix pthread sjp nway fmt)
if(UNIX AND NOT APPLE)
    target_link_libraries(logifix -static-libgcc -static-libstdc++)
endif()

### TESTS

enable_testing()
add_subdirectory(test)
