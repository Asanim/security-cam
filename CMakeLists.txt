cmake_minimum_required(VERSION 3.1)
project (SecurityCamera)

# Print CMake version
message("CMake version: ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
message("Full CMake version: ${CMAKE_VERSION}")

option(TRACE OFF)
set(CMAKE_INSTALL_MESSAGE LAZY)
set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(INSTALL_DIR ${CMAKE_BINARY_DIR}/external/install)

include(scripts/cmake/aws-sdk-cpp.cmake)
include(scripts/cmake/aws-iot-device-sdk-cpp-v2.cmake)
include(scripts/cmake/can-utils.cmake)
include(scripts/cmake/sqlite.cmake)
include(scripts/cmake/opencv.cmake)
include(scripts/cmake/zlib.cmake)

add_subdirectory(src)
