cmake_minimum_required(VERSION 3.18)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

project(logifix)

set(CMAKE_CXX_STANDARD 17)

set(BUILD_SHARED_LIBS OFF)
set(BUILD_THREADSAFE ON)
set(REGEX_BACKEND builtin)
set(USE_BUNDLED_ZLIB ON)
set(USE_HTTPS OFF)
set(USE_SSH OFF)
set(USE_NTLMCLIENT OFF)
set(BUILD_CLAR OFF)
add_subdirectory(vendor/libgit2)

add_subdirectory(sjp)

add_executable(logifix cli/main.cpp program.cpp)

target_link_libraries(logifix git2 sjp)
