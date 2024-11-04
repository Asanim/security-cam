include(ExternalProject)

message(STATUS "Added aws-iot-device-sdk-cpp-v2 to external submodules")

# ------------------------------------------------------------------------------
# aws-iot-device-sdk-cpp-v2
# ------------------------------------------------------------------------------
ExternalProject_Add(
  aws-iot-device-sdk-cpp-v2
  GIT_REPOSITORY https://github.com/aws/aws-iot-device-sdk-cpp-v2
  GIT_SHALLOW 1
  GIT_TAG v1.33.0
  CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
  -DCMAKE_BUILD_TYPE=Release
  -DBUILD_SHARED_LIBS=ON
  -DCMAKE_INSTALL_PREFIX=
  -DUSE_OPENSSL=ON
  PREFIX ${CMAKE_BINARY_DIR}/external/aws-iot-device-sdk-cpp-v2/prefix
  TMP_DIR ${CMAKE_BINARY_DIR}/external/aws-iot-device-sdk-cpp-v2/tmp
  STAMP_DIR ${CMAKE_BINARY_DIR}/external/aws-iot-device-sdk-cpp-v2/stamp
  DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/external/aws-iot-device-sdk-cpp-v2/download
  SOURCE_DIR ${CMAKE_BINARY_DIR}/external/aws-iot-device-sdk-cpp-v2/src
  BINARY_DIR ${CMAKE_BINARY_DIR}/external/aws-iot-device-sdk-cpp-v2/build
  INSTALL_DIR ${CMAKE_BINARY_DIR}/external/install
  INSTALL_COMMAND make DESTDIR=${INSTALL_DIR} install
  UPDATE_DISCONNECTED 1
  BUILD_ALWAYS 0
)
