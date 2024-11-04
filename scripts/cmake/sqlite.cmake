include(ExternalProject)

message (STATUS "Added sqlite to external submodules")

# ------------------------------------------------------------------------------
# sqlite
# ------------------------------------------------------------------------------
ExternalProject_Add(
    sqlite
    URL                 https://www.sqlite.org/2022/sqlite-autoconf-3400100.tar.gz
    PREFIX              ${CMAKE_BINARY_DIR}/external/sqlite/prefix
    TMP_DIR             ${CMAKE_BINARY_DIR}/external/sqlite/tmp
    STAMP_DIR           ${CMAKE_BINARY_DIR}/external/sqlite/stamp
    DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}/external/sqlite/download
    SOURCE_DIR          ${CMAKE_BINARY_DIR}/external/sqlite/src
    BUILD_IN_SOURCE     TRUE
    CONFIGURE_COMMAND   ./configure --build x86_64-pc-linux-gnu --host=${TARGET_CROSS} --prefix=/${TARGET_PREFIX} CFLAGS=-DSQLITE_ENABLE_COLUMN_METADATA=1
    INSTALL_COMMAND     make DESTDIR=${CMAKE_BINARY_DIR}/external/install install
    UPDATE_DISCONNECTED 1
    BUILD_ALWAYS        0
)

set(SQLITE_INCLUDE_DIR ${CMAKE_BINARY_DIR}/external/install/include)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm-linux-gnueabihf")
    set(SQLITE_LIBRARY_DIR ${CMAKE_BINARY_DIR}/external/install/lib)
else()
    set(SQLITE_LIBRARY_DIR ${CMAKE_BINARY_DIR}/usr/lib/x86_64-linux-gnu)
endif()
