
include(ExternalProject)

message(STATUS "Added coral-tensorflow-lite to external submodules")

set(ARMCC_FLAGS "-march=armv7-a -funsafe-math-optimizations")
set(ARMCC_PREFIX $ENV{HOME}/toolchains/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-)

message(STATUS "ARMCC_FLAGS: ${ARMCC_FLAGS}")
message(STATUS "ARMCC_PREFIX: ${ARMCC_PREFIX}")

message(STATUS "DCMAKE_C_COMPILER: ${ARMCC_PREFIX}gcc")
message(STATUS "DCMAKE_CXX_COMPILER: ${ARMCC_PREFIX}g++")


# ------------------------------------------------------------------------------
# coral-tensorflow-lite
# ------------------------------------------------------------------------------
ExternalProject_Add(
    coral-tensorflow-lite
    GIT_REPOSITORY https://github.com/tensorflow/tensorflow.git
    GIT_SHALLOW 1
    GIT_SUBMODULES_RECURSE 1
    GIT_TAG v2.16.1
    CMAKE_ARGS
    TMP_DIR ${CMAKE_BINARY_DIR}/external/coral-tensorflow-lite/tmp
    STAMP_DIR ${CMAKE_BINARY_DIR}/external/coral-tensorflow-lite/stamp
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/external/coral-tensorflow-lite/download
    SOURCE_DIR ${CMAKE_BINARY_DIR}/external/coral-tensorflow-lite/src/
    BINARY_DIR ${CMAKE_BINARY_DIR}/external/coral-tensorflow-lite/build
    INSTALL_COMMAND make DESTDIR=${CMAKE_BINARY_DIR}/external/install install
    CONFIGURE_COMMAND
    cmake -DCMAKE_C_COMPILER=/home/sam/toolchains/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
    -DCMAKE_CXX_COMPILER=/home/sam/toolchains/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++
    -DCMAKE_C_FLAGS="-march=armv7-a"
    -DCMAKE_CXX_FLAGS="-march=armv7-a"
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
    -DCMAKE_SYSTEM_NAME=Linux
    -DTFLITE_ENABLE_XNNPACK=OFF
    -DTFLITE_HOST_TOOLS_DIR=/home/sam/sentinel_vision_pe-sv-1200/build/external/install/bin
    -DFlatBuffers_DIR=/home/sam/sentinel_vision_pe-sv-1200/build/external/install/lib/cmake/flatbuffers
    -DCMAKE_SYSTEM_PROCESSOR=armv7
    ../src/tensorflow/lite/
    UPDATE_DISCONNECTED 1
    BUILD_ALWAYS 0
)

add_dependencies(coral-tensorflow-lite flatc libusb)

# Custom command to copy libcoral-tensorflow-lite.a to the install directory
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/external/install/lib/libcoral-tensorflow-lite.a
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/external/install/lib
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/external/coral-tensorflow-lite/build/libcoral-tensorflow-lite.a
    ${CMAKE_BINARY_DIR}/external/install/lib/
    COMMENT "Quickfix: Copying libcoral-tensorflow-lite library to install directory"

    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/external/tensorflow-lite/build/_deps/farmhash-build/libfarmhash.a
    ${CMAKE_BINARY_DIR}/external/install/lib/
    COMMENT "Quickfix: Copying farmhash library to install directory"
    DEPENDS ${CMAKE_BINARY_DIR}/external/coral-tensorflow-lite/build/libcoral-tensorflow-lite.a
)

# Custom target to trigger the copy command
add_custom_target(copy_libtensorflow_lite ALL
    DEPENDS ${CMAKE_BINARY_DIR}/external/install/lib/libcoral-tensorflow-lite.a)

# Ensure copy target depends on the coral-tensorflow-lite build
add_dependencies(copy_libtensorflow_lite coral-tensorflow-lite)
