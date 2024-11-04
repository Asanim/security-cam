# Include the ExternalProject module
include(ExternalProject)

# Message to indicate that OpenCV is being added as an external project
message(STATUS "Adding OpenCV 4 as an external project")

# Define the external project for OpenCV
ExternalProject_Add(opencv
    PREFIX ${CMAKE_BINARY_DIR}/opencv
    GIT_REPOSITORY https://github.com/opencv/opencv.git
    GIT_TAG 4.x # Replace with specific version tag, e.g., 4.5.5
    CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/opencv/install
    -DBUILD_LIST=core,imgproc,imgcodecs,highgui # Choose modules as required
    -DBUILD_SHARED_LIBS=ON
    -DBUILD_EXAMPLES=OFF
    -DBUILD_TESTS=OFF
    -DBUILD_PERF_TESTS=OFF
    -DCMAKE_BUILD_TYPE=Release
    -DBUILD_SHARED_LIBS=ON
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external/install
    -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/external/install
    -DCMAKE_BUILD_TYPE=Release
    PREFIX ${CMAKE_BINARY_DIR}/external/opencv/prefix
    TMP_DIR ${CMAKE_BINARY_DIR}/external/opencv/tmp
    STAMP_DIR ${CMAKE_BINARY_DIR}/external/opencv/stamp
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/external/opencv/download
    SOURCE_DIR ${CMAKE_BINARY_DIR}/external/opencv/src
    BINARY_DIR ${CMAKE_BINARY_DIR}/external/opencv/build
    INSTALL_DIR ${CMAKE_BINARY_DIR}/external/install
    UPDATE_DISCONNECTED 1
    BUILD_ALWAYS 0
)

add_dependencies(opencv zlib)

