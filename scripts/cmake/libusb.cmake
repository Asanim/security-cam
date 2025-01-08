include(ExternalProject)

set(CCPREFIX ${TOOL_PATH}/bin/)

MESSAGE(STATUS "CCPREFIX: ${CCPREFIX}")

# Define the external project for libusb
ExternalProject_Add(
    libusb
    GIT_REPOSITORY https://github.com/libusb/libusb.git
    GIT_TAG v1.0.26 # Specify the required version or branch
    SOURCE_DIR ${CMAKE_BINARY_DIR}/external/libusb/src
    TMP_DIR ${CMAKE_BINARY_DIR}/external/libusb/tmp
    STAMP_DIR ${CMAKE_BINARY_DIR}/external/libusb/stamp
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/external/libusb/download
    BUILD_IN_SOURCE TRUE
    CONFIGURE_COMMAND
    ./bootstrap.sh &&
    ./configure --host=arm-linux-gnueabihf
    --prefix=${CMAKE_BINARY_DIR}/external/install
    CC=${CMAKE_C_COMPILER}
    --enable-udev=false
    # Build command using multiple cores
    BUILD_COMMAND make -j$(nproc) DESTDIR=${CMAKE_BINARY_DIR}/external/install
    UPDATE_DISCONNECTED 1
    BUILD_ALWAYS 0
)
