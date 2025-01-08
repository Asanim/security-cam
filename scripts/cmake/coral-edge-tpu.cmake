include(ExternalProject)

message(STATUS "Added EdgeTPU runtime to external submodules")

# ------------------------------------------------------------------------------
# EdgeTPU Runtime
# ------------------------------------------------------------------------------
ExternalProject_Add(
    edgetpu-runtime
    URL                 https://github.com/google-coral/libedgetpu/releases/download/release-grouper/edgetpu_runtime_20221024.zip
    PREFIX              ${CMAKE_BINARY_DIR}/external/edgetpu/prefix
    TMP_DIR             ${CMAKE_BINARY_DIR}/external/edgetpu/tmp
    STAMP_DIR           ${CMAKE_BINARY_DIR}/external/edgetpu/stamp
    DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}/external/edgetpu/download
    SOURCE_DIR          ${CMAKE_BINARY_DIR}/external/edgetpu/src
    BUILD_IN_SOURCE     TRUE
    CONFIGURE_COMMAND   ""  # No configure step needed
    BUILD_COMMAND       ""  # No build step needed
    INSTALL_COMMAND     ""  # No install step needed
    UPDATE_DISCONNECTED 1
    BUILD_ALWAYS        0
)
# Step to copy the shared library to the install directory
add_custom_command(
    TARGET edgetpu-runtime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/external/edgetpu/src/libedgetpu/direct/armv7a
        ${CMAKE_BINARY_DIR}/external/install/lib/
    COMMENT "Copying EdgeTPU shared library to install directory"
)

# Step to copy the header files to the install directory
add_custom_command(
    TARGET edgetpu-runtime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/external/edgetpu/src/libedgetpu
        ${CMAKE_BINARY_DIR}/external/install/include
    COMMENT "Copying EdgeTPU header files to install directory"
)

# Set the include and library directories based on architecture
set(EDGETPU_INCLUDE_DIR ${CMAKE_BINARY_DIR}/external/install/libedgetpu)

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm-linux-gnueabihf")
    set(EDGETPU_LIBRARY_DIR ${CMAKE_BINARY_DIR}/external/install/lib/armv7a)
else()
    set(EDGETPU_LIBRARY_DIR ${CMAKE_BINARY_DIR}/external/install/lib)
endif()

message(STATUS "EdgeTPU runtime added and extracted successfully.")
