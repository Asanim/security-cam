sudo apt install -y libnvinfer-dev libnvinfer-plugin-dev libnvonnxparsers-dev libnvparsers-dev tensorrt tensorrt-libs python3-libnvinfer-dev libjsoncpp-dev cuda-cudart-dev-11-4 cuda-cupti-dev-11-4 cuda-driver-dev-11-4 cuda-libraries-dev-11-4 cuda-nvml-dev-11-4 cuda-nvrtc-dev-11-4 libcusolver-dev-11-4

# Download the SSD model.engine file
wget -O ssd_mobilenet_v2_coco.engine https://github.com/jkjung-avt/tensorrt_demos/releases/download/v1.0/ssd_mobilenet_v2_coco.engine