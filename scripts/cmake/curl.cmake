include(ExternalProject)

message(STATUS "Added curl to external submodules")
# ------------------------------------------------------------------------------
# curl
# ------------------------------------------------------------------------------
ExternalProject_Add(
    curl
    GIT_REPOSITORY https://github.com/curl/curl
    GIT_SHALLOW 1
    GIT_TAG curl-8_0_1
    CMAKE_ARGS
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external/install
        -DZLIB_LIBRARY=${CMAKE_BINARY_DIR}/external/install/lib/libz.so
        -DZLIB_INCLUDE_DIR=${CMAKE_BINARY_DIR}/external/install/include
        -DOPENSSL_ROOT_DIR=${CMAKE_BINARY_DIR}/external/install
        -DOPENSSL_INCLUDE_DIR=${CMAKE_BINARY_DIR}/external/install/include
        -DOPENSSL_CRYPTO_LIBRARY=${CMAKE_BINARY_DIR}/external/install/lib/libcrypto.so
        -DOPENSSL_SSL_LIBRARY=${CMAKE_BINARY_DIR}/external/install/lib/libssl.so
    PREFIX ${CMAKE_BINARY_DIR}/external/curl/prefix
    TMP_DIR ${CMAKE_BINARY_DIR}/external/curl/tmp
    STAMP_DIR ${CMAKE_BINARY_DIR}/external/curl/stamp
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/external/curl/download
    SOURCE_DIR ${CMAKE_BINARY_DIR}/external/curl/src
    BINARY_DIR ${CMAKE_BINARY_DIR}/external/curl/build
    UPDATE_DISCONNECTED 1
    BUILD_ALWAYS 0
)

add_dependencies(curl crypto zlib)
