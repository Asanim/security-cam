# SecurityCam

This project uses TensorRT to perform object detection on a webcam stream using a Jetson Orin Nano.

Please now create a webstream server class which takes each frame and streams it to a port on localhost / default ip of the edge device
## Prerequisites

- Jetson Orin Nano
- OpenCV
- TensorRT

## Build

```bash
mkdir build
cd build
cmake ..
make
```

## Run

```bash
./SecurityCam
```