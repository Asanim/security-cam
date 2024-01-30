#!/bin/bash

# Exit on any error
set -e

SCRIPT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
INSTALL_DIR="/opt/sdk_workspace/"
BUILD_PATH=${SCRIPT_PATH}/../build/sdk-workspace

# Set the toolchain PATH
# export PATH=$PATH:/tools/toolchain/gcc-sigmastar-9.1.0-2020.07-x86_64_arm-linux-gnueabihf/bin:/tools/toolchain/gcc-sigmastar-9.1.0-2020.07-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/libc/usr/include:$PATH

rm -rf ${BUILD_PATH}
mkdir -p ${BUILD_PATH}

cd ${BUILD_PATH}

# Clone and build s2n
git clone https://github.com/awslabs/s2n.git
cd s2n
cmake .
make
sudo make install
cd ..

# Clone the AWS IoT Device SDK repository
git clone --recursive https://github.com/aws/aws-iot-device-sdk-cpp-v2.git

# Ensure all submodules are properly updated
cd aws-iot-device-sdk-cpp-v2
git submodule update --init --recursive
cd ..

# Make a build directory for the SDK. Can use any name.
# If working with multiple SDKs, using a SDK-specific name is helpful.
rm -rf aws-iot-device-sdk-cpp-v2-build
mkdir aws-iot-device-sdk-cpp-v2-build
cd aws-iot-device-sdk-cpp-v2-build

echo "$PWD"

# Configure the build, including the path to s2n
cmake ../aws-iot-device-sdk-cpp-v2 \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DS2N_LIBRARY_PATH="/usr/local/lib/libs2n.so"  # Adjust the path as needed

# Build and install the AWS IoT Device SDK
cmake --build . --target install

# Clean up temporary build files
rm -rf ${BUILD_PATH}

# Apply hotfix (if needed)
# find /opt/sdk_workspace/lib/ -type f -name "*.cmake" -exec sed -i 's/message(FATAL_ERROR/message(STATUS/g' {} \;

echo "AWS IoT Device SDK C++ version 2 and s2n have been installed to $INSTALL_DIR and /usr/local"
