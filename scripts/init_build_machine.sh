#!/usr/bin/bash

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

SCRIPT_PATH="$(dirname -- "${BASH_SOURCE[0]}")"

echo "Installing essential dependencies..."
apt-get update
apt-get install -y cmake g++ make libopencv-dev unzip wget git kmod curl sqlite3 python3-pip python-is-python3

# Install gdk - greengrass development kit
apt-get install -y jq gettext-base 
pip3 install -U git+https://github.com/aws-greengrass/aws-greengrass-gdk-cli.git@v1.3.0


# Check if AWS CLI is already installed
if ! command -v aws &>/dev/null; then
  # AWS CLI is not installed, so download and install it
  echo "AWS CLI is not installed. Installing..."
  ${SCRIPT_PATH}/install_aws_cli.sh
else
    # AWS CLI is already installed
    echo "AWS CLI is already installed."
fi

# crosscompile and install the aws sdk to the system
if [ ! -d "/opt/sdk_workspace" ]; then
  echo "AWS SDK is not installed, "
  ${SCRIPT_PATH}/install_aws_sdk.sh
else
    echo "AWS c++ SDK is already installed."
fi