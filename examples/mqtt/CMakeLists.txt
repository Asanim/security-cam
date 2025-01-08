
# The following is a quickfix as somewhere in cmake, CMAKE_MAJOR_VERSION CMAKE_MINOR_VERSION and CMAKE_PATCH_VERSION are set incorrectly causing the aws libraries to fail
# Print CMake version
message("src CMake version: ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
message("src Full CMake version: ${CMAKE_VERSION}")

# Get major, minor, and patch versions from CMAKE_VERSION
string(REGEX MATCH "^[0-9]+" CMAKE_MAJOR_VERSION "${CMAKE_VERSION}")
string(REGEX MATCH "\\.[0-9]+\\." CMAKE_MINOR_VERSION "${CMAKE_VERSION}")
string(REGEX MATCH "\\.[0-9]+$" CMAKE_PATCH_VERSION "${CMAKE_VERSION}")

# Remove leading and trailing dots
string(REGEX REPLACE "\\." "" CMAKE_MINOR_VERSION "${CMAKE_MINOR_VERSION}")
string(REGEX REPLACE "\\." "" CMAKE_PATCH_VERSION "${CMAKE_PATCH_VERSION}")

# Print the extracted versions
message("CMAKE_VERSION: ${CMAKE_VERSION}")
message("CMAKE_MAJOR_VERSION: ${CMAKE_MAJOR_VERSION}")
message("CMAKE_MINOR_VERSION: ${CMAKE_MINOR_VERSION}")
message("CMAKE_PATCH_VERSION: ${CMAKE_PATCH_VERSION}")

## Set AWS include and library directories
set(AWS_SDK_DIR /opt/sdk_workspace)

list(APPEND CMAKE_PREFIX_PATH ${AWS_SDK_DIR}/lib/)
set(AWS_LIBRARY_DIR ${AWS_SDK_DIR}/lib/)
set(AWS_INCLUDE_DIR ${AWS_SDK_DIR}/include/)

set(crypto_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include/)
set(crypto_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.so)

find_package(aws-crt-cpp REQUIRED)
find_package(EventstreamRpc-cpp REQUIRED)
find_package(GreengrassIpc-cpp REQUIRED)

## Datalogging executable
add_executable(datalogging main_mqtt_standalone.cc)

set_target_properties(datalogging PROPERTIES
        LINKER_LANGUAGE CXX
        CXX_STANDARD 11) 

target_link_libraries(datalogging AWS::GreengrassIpc-cpp)