cmake_minimum_required(VERSION 3.1)
project (DeferDeployment)

set(EXECUTABLE_NAME main)

add_executable(${EXECUTABLE_NAME} main.cc)

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
        LINKER_LANGUAGE CXX
        CXX_STANDARD 17) 

## Set AWS include and library directories
set(AWS_SDK_DIR /opt/sdk_workspace_64)

list(APPEND CMAKE_PREFIX_PATH ${AWS_SDK_DIR}/lib/)
set(AWS_LIBRARY_DIR ${AWS_SDK_DIR}/lib/)
set(AWS_INCLUDE_DIR ${AWS_SDK_DIR}/include/)

set(crypto_INCLUDE_DIR ${AWS_SDK_DIR}/include/)
set(crypto_LIBRARY ${AWS_SDK_DIR}/lib/libcrypto.so)

find_package(aws-crt-cpp REQUIRED)
find_package(EventstreamRpc-cpp REQUIRED)
find_package(GreengrassIpc-cpp REQUIRED)

target_link_libraries(${EXECUTABLE_NAME} AWS::GreengrassIpc-cpp)
