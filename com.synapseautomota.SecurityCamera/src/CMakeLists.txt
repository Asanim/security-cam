cmake_minimum_required(VERSION 3.1)
project (SecurityCamera)

add_executable(${PROJECT_NAME}
    main.cc
    SubscribeUpdatesHandler.cpp
    IpcClientLifecycleHandler.cpp
    IpcClientPublisher.cpp
    Logger.cpp
    OpenCVUtils.cpp
    SubscribeResponseHandler.cpp
    alg_image_detect.cpp
    alg_standalone.cpp
    fifo_queue.cpp
    inference_rknn.cpp
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
