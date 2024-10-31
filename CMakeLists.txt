cmake_minimum_required(VERSION 3.1)
project (SecurityCamera)

# Print CMake version
message("CMake version: ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
message("Full CMake version: ${CMAKE_VERSION}")

option(TRACE OFF)
set(CMAKE_INSTALL_MESSAGE LAZY)
set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(INSTALL_DIR ${CMAKE_BINARY_DIR}/external/install)

add_executable(${PROJECT_NAME}
    src/main.cc
    # src/SubscribeUpdatesHandler.cpp
    # src/IpcClientLifecycleHandler.cpp
    # src/IpcClientPublisher.cpp
    # src/Logger.cpp
    # src/OpenCVUtils.cpp
    # src/SubscribeResponseHandler.cpp
    # src/alg_image_detect.cpp
    # src/alg_standalone.cpp
    # src/fifo_queue.cpp
    # src/inference_rknn.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        LINKER_LANGUAGE CXX
        CXX_STANDARD 11) 

set(AWS_SDK_DIR /opt/sdk_workspace)

# list(APPEND CMAKE_PREFIX_PATH ${AWS_SDK_DIR}/lib/)
set(AWS_LIBRARY_DIR ${AWS_SDK_DIR}/lib/)
set(AWS_INCLUDE_DIR ${AWS_SDK_DIR}/include/)

# Set include and library directories
set(crypto_INCLUDE_DIR ${AWS_SDK_DIR}/include/)
set(crypto_LIBRARY ${AWS_SDK_DIR}/lib/libcrypto.a)
set(crypto_STATIC_LIBRARY ${AWS_SDK_DIR}/lib/libcrypto.a)

find_package(aws-crt-cpp REQUIRED)
find_package(EventstreamRpc-cpp REQUIRED)
find_package(GreengrassIpc-cpp REQUIRED)
target_link_libraries(${PROJECT_NAME} AWS::GreengrassIpc-cpp ${crypto_LIBRARY})
