#!/bin/bash
SCRIPT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_PATH=${SCRIPT_PATH}/../build/

# create build path
cd $BUILD_PATH

# Download the AWS CLI installation package
curl "https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip" -o "awscliv2.zip"

# Unzip the installation package
unzip awscliv2.zip

# Run the installation script
./aws/install --update

# Verify the installation
if [ $? -eq 0 ]; then
    echo "AWS CLI installation successful."
else
    echo "AWS CLI installation failed."
fi

# Remove the install directory
rm -rf ./aws
rm -rf ./awscliv2.zip
cd ${SCRIPT_PATH}
