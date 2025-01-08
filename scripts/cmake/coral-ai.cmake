include(ExternalProject)

message(STATUS "Added coral-ai to external submodules")

# ------------------------------------------------------------------------------
# coral-ai
# ------------------------------------------------------------------------------
ExternalProject_Add(
    coral-ai
    GIT_REPOSITORY https://github.com/google-coral/libcoral.git
    GIT_SHALLOW 1
    GIT_SUBMODULES_RECURSE 1
    GIT_TAG master
    PREFIX ${CMAKE_BINARY_DIR}/external/coral-ai/prefix
    TMP_DIR             ${CMAKE_BINARY_DIR}/external/coral-ai/tmp
    STAMP_DIR           ${CMAKE_BINARY_DIR}/external/coral-ai/stamp
    DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}/external/coral-ai/download
    SOURCE_DIR          ${CMAKE_BINARY_DIR}/external/coral-ai/src
    CONFIGURE_COMMAND   make DOCKER_IMAGE=ubuntu:18.04 DOCKER_CPUS="armv7a" DOCKER_TARGETS=docker-build
    BUILD_IN_SOURCE TRUE
    BUILD_COMMAND       env USE_BAZEL_VERSION=4.0.0 bazel --version  make CPU=armv7a neon=on -j4
    INSTALL_COMMAND DESTDIR=${CMAKE_BINARY_DIR}/external/install install
    UPDATE_DISCONNECTED 1
    BUILD_ALWAYS 0
)
