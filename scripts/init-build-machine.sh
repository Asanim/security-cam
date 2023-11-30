# Install necessary libraries
sudo apt update
sudo apt install -y libnvinfer-dev libnvinfer-plugin-dev libnvonnxparsers-dev libnvparsers-dev tensorrt tensorrt-libs python3-libnvinfer-dev libjsoncpp-dev cuda-cudart-dev-11-4 cuda-cupti-dev-11-4 cuda-driver-dev-11-4 cuda-libraries-dev-11-4 cuda-nvml-dev-11-4 cuda-nvrtc-dev-11-4 libcusolver-dev-11-4 libpoco-dev libpoco-doc

sudo add-apt-repository ppa:pistache+team/unstable
sudo apt update
sudo apt install -y libpistache-dev

# Download the SSD model.engine file
wget -O ssd_mobilenet_v2_coco.engine https://github.com/jkjung-avt/tensorrt_demos/releases/download/v1.0/ssd_mobilenet_v2_coco.engine

# Clone YOLOv7 repository
git clone https://github.com/WongKinYiu/yolov7.git $HOME/security-cam/yolov7

# Clone YOLOv8 repository
git clone https://github.com/ultralytics/yolov8.git $HOME/security-cam/yolov8

