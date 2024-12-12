include(ExternalProject)

message (STATUS "Added can-utils to external submodules")

# ------------------------------------------------------------------------------
# can-utils
# ------------------------------------------------------------------------------
ExternalProject_Add(
    can-utils
    GIT_REPOSITORY      https://github.com/linux-can/can-utils
    GIT_SHALLOW         1
    CMAKE_ARGS          -DCMAKE_INSTALL_MESSAGE=LAZY
    GIT_TAG             v2021.08.0  
    PREFIX              ${CMAKE_BINARY_DIR}/external/can-utils/prefix
    TMP_DIR             ${CMAKE_BINARY_DIR}/external/can-utils/tmp
    STAMP_DIR           ${CMAKE_BINARY_DIR}/external/can-utils/stamp
    DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}/external/can-utils/download
    SOURCE_DIR          ${CMAKE_BINARY_DIR}/external/can-utils/src
    BINARY_DIR          ${CMAKE_BINARY_DIR}/external/can-utils/build
    INSTALL_COMMAND     make DESTDIR=${CMAKE_BINARY_DIR}/external/install install
    UPDATE_DISCONNECTED 1
    BUILD_ALWAYS        0
)

set(CAN_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/external/can-utils/src/include/)