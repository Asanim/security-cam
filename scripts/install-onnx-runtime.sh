#!/usr/bin/bash

wget https://github.com/microsoft/onnxruntime/releases/download/v1.20.0/onnxruntime-linux-x64-1.20.0.tgz

# Move libraries
sudo cp -r onnxruntime-linux-x64-1.20.0/lib/* /usr/local/lib/

# Move include headers
sudo cp -r onnxruntime-linux-x64-1.20.0/include/* /usr/local/include/

# Move pkgconfig files
sudo cp -r onnxruntime-linux-x64-1.20.0/lib/pkgconfig/* /usr/local/lib/pkgconfig/