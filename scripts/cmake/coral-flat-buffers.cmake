include(ExternalProject)

# ------------------------------------------------------------------------------
# FlatBuffers External Project
# ------------------------------------------------------------------------------
message(STATUS "Adding FlatBuffers to external submodules...")

ExternalProject_Add(
    flat-buffers
    GIT_REPOSITORY https://github.com/google/flatbuffers.git
    GIT_TAG v24.3.25 # Specify the compatible FlatBuffers version
    CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=Release
    -DBUILD_SHARED_LIBS=ON # Enable shared library build
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external/install
    -DFLATBUFFERS_BUILD_TESTS=OFF # Disable tests for faster builds
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    BUILD_BYPRODUCTS
    ${CMAKE_BINARY_DIR}/external/install/bin/flatc
    PREFIX ${CMAKE_BINARY_DIR}/external/flat-buffers/prefix
    TMP_DIR ${CMAKE_BINARY_DIR}/external/flat-buffers/tmp
    STAMP_DIR ${CMAKE_BINARY_DIR}/external/flat-buffers/stamp
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/external/flat-buffers/download
    SOURCE_DIR ${CMAKE_BINARY_DIR}/external/flat-buffers/src
    BINARY_DIR ${CMAKE_BINARY_DIR}/external/flat-buffers/build
    INSTALL_COMMAND
    make DESTDIR=${CMAKE_BINARY_DIR}/external/install install
)

# Define the flatc executable as an imported target
add_executable(flatc IMPORTED)
set_target_properties(flatc PROPERTIES
    IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/external/install/bin/flatc
)

# Ensure dependent projects wait for FlatBuffers to build
add_dependencies(flatc flat-buffers)
