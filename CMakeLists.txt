cmake_minimum_required(VERSION 3.10)
project(SecurityCam)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Find TensorRT
find_package(TensorRT REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS} ${TENSORRT_INCLUDE_DIRS})

# Add executable
add_executable(SecurityCam main.cpp)

# Link libraries
target_link_libraries(SecurityCam ${OpenCV_LIBS} ${TENSORRT_LIBRARIES})
