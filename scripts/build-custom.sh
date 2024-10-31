#!/usr/bin/bash

# Cleanup: Remove existing build directory
rm -rf src/build

# Create a new build directory
mkdir src/build

# Navigate to the build directory
cd src/build

# Configure the project using CMake
cmake ..

# Build the project using Make
make
