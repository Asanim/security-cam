cmake_minimum_required(VERSION 3.10)

project(SecurityCam)
enable_language(CUDA)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g -fpic -fpie -fpermissive -std=c++14 -pthread")

# Find OpenCV
find_package(OpenCV REQUIRED)

# Find CUDA
find_package(CUDA REQUIRED)

# Find Poco
find_package(Poco REQUIRED COMPONENTS Net Util)

# Find Pistache
find_package(Pistache REQUIRED)

# Include directories
include_directories(${CUDA_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${Poco_INCLUDE_DIRS} ${PISTACHE_INCLUDE_DIRS})

# Add system-specific include directories
MESSAGE(STATUS "CMAKE_HOST_SYSTEM_PROCESSOR is ${CMAKE_HOST_SYSTEM_PROCESSOR}")
if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    include_directories(/usr/include/aarch64-linux-gnu/) # for Jetson
elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    include_directories(/usr/lib/x86_64-linux-gnu/)
endif()

# Add executable
add_executable(SecurityCam main.cpp yolov7.cpp yolov8.cpp webstream_server.cpp rest_api_server.cpp detection_pipeline.cpp)

# Link libraries
if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    link_directories(/usr/lib/aarch64-linux-gnu/)
elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    link_directories(/usr/lib/x86_64-linux-gnu/)
endif()

link_directories(/usr/lib/ /usr/local/lib/ /usr/local/cuda/lib64/ /usr/local/cuda/lib/)
target_link_libraries(SecurityCam ${OpenCV_LIBS} ${CUDA_LIBRARIES} nvinfer nvinfer_plugin nvparsers nvonnxparser ${Poco_LIBRARIES} Pistache::Pistache sqlite3)
