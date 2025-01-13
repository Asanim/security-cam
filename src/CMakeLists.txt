
message("Application Version " ${VERSION_STRING})

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/awsiot
)

set(LIBRARIES_TO_LINK
    awsiot
)

set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/web-streamer.cc
    ${CMAKE_SOURCE_DIR}/src/camera-handler.cc
    ${CMAKE_SOURCE_DIR}/src/alg/inference-coral.cc
)

find_package(OpenCV REQUIRED)

set(AWS_IOT_SOURCES
    awsiot/mqtt_handler.cpp
)

## Add the AWS Library 
add_library(awsiot STATIC ${AWS_IOT_SOURCES})
add_dependencies(awsiot
    aws-sdk-cpp
    aws-iot-device-sdk-cpp-v2
)

target_include_directories(awsiot PRIVATE
    ${CMAKE_BINARY_DIR}/external/install/include
    ${OPENSSL_ROOT_DIR}/include # OpenSSL includes
    ${CMAKE_SOURCE_DIR}/src/awsiot
)

target_link_directories(awsiot PRIVATE
    ${CMAKE_BINARY_DIR}/external/install/lib/ # LibCurl
    ${OPENSSL_ROOT_DIR}/lib # OpenSSL libs
)

target_link_libraries(awsiot
    pthread
    ${AWS_SDK_CPP_SOURCES}
)

## Security Camera executable
add_executable(${PROJECT_NAME}
    main.cc
    ${SOURCES}
)

add_dependencies(${PROJECT_NAME} opencv coral-ai)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_BINARY_DIR}/external/install/include
    ${CMAKE_BINARY_DIR}/external/install/include/opencv4
)
