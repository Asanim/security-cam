include(ExternalProject)

message(STATUS "Added ZLib to external submodules")

# ------------------------------------------------------------------------------
# zlib
# ------------------------------------------------------------------------------
ExternalProject_Add(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib
    GIT_SHALLOW 1
    GIT_TAG v1.3.1
    CMAKE_ARGS
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external/install
    PREFIX ${CMAKE_BINARY_DIR}/external/zlib/prefix
    TMP_DIR ${CMAKE_BINARY_DIR}/external/zlib/tmp
    STAMP_DIR ${CMAKE_BINARY_DIR}/external/zlib/stamp
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/external/zlib/download
    SOURCE_DIR ${CMAKE_BINARY_DIR}/external/zlib/src
    BINARY_DIR ${CMAKE_BINARY_DIR}/external/zlib/build
    UPDATE_DISCONNECTED 1
    BUILD_ALWAYS 0
)
