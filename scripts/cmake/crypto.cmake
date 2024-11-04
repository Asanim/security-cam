include(ExternalProject)

message (STATUS "Added crypto to external submodules")

# ------------------------------------------------------------------------------
# crypto
# ------------------------------------------------------------------------------
ExternalProject_Add(
    crypto
    GIT_REPOSITORY      https://github.com/openssl/openssl
    GIT_SHALLOW         1
    GIT_TAG             openssl-3.0.7  
    PREFIX              ${CMAKE_BINARY_DIR}/external/crypto/prefix
    TMP_DIR             ${CMAKE_BINARY_DIR}/external/crypto/tmp
    STAMP_DIR           ${CMAKE_BINARY_DIR}/external/crypto/stamp
    DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}/external/crypto/download
    SOURCE_DIR          ${CMAKE_BINARY_DIR}/external/crypto/src
    BUILD_IN_SOURCE     TRUE
    CONFIGURE_COMMAND   ./Configure linux-generic32 no-asm shared --cross-compile-prefix=${TARGET_CROSS}-  --prefix=${INSTALL_DIR} --openssldir=${INSTALL_DIR} -DCMAKE_BUILD_TYPE=Release
    BUILD_COMMAND       make $(nproc)
    INSTALL_COMMAND     make install
    UPDATE_DISCONNECTED 1
    BUILD_ALWAYS        0
)

add_dependencies(crypto zlib)
