include(ExternalProject)

message(STATUS "Added rknpu to external submodules")
# ------------------------------------------------------------------------------
# rknpu
# ------------------------------------------------------------------------------
ExternalProject_Add(
    rknpu
    GIT_REPOSITORY https://github.com/airockchip/RK3399Pro_npu.git
    GIT_SHALLOW 1
    GIT_TAG main
    CMAKE_ARGS
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external/install
    PREFIX ${CMAKE_BINARY_DIR}/external/rknpu/prefix
    TMP_DIR ${CMAKE_BINARY_DIR}/external/rknpu/tmp
    STAMP_DIR ${CMAKE_BINARY_DIR}/external/rknpu/stamp
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/external/rknpu/download
    SOURCE_DIR ${CMAKE_BINARY_DIR}/external/rknpu/src
    BINARY_DIR ${CMAKE_BINARY_DIR}/external/rknpu/build
    UPDATE_DISCONNECTED 1
    BUILD_ALWAYS 0
)