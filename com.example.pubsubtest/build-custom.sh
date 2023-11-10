#!/usr/bin/bash
rm -rf src/build
mkdir src/build
cd src/build

cmake .. 
make 

tar -czf source.tar.gz datalogging
cp source.tar.gz ../../greengrass-build/artifacts/$COMPONENT_NAME/$COMPONENT_VERSION
