#!/usr/bin/bash\
rm -rf src/build
mkdir src/build
cd src/build

cmake .. 
make 
cp datalogging ../../greengrass-build/artifacts/$COMPONENT_NAME/$COMPONENT_VERSION