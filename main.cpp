#include <iostream>
#include <string>
#include <memory>
#include <cuda_runtime.h>
#include "NvInfer.h"
#include <vector>
#include <NvInferPlugin.h>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class Logger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) noexcept override {
        // suppress info-level messages
        if (severity <= Severity::kWARNING) {
            std::cout << msg << std::endl;
        }
    }
} gLogger;

const int INPUT_SIZE = 300 * 300 * 3; // Adjusted for SSD input size
const int OUTPUT_SIZE = 1000; // Adjust as needed for SSD output size

void drawDetections(cv::Mat& frame, const float* detections, int numDetections) {
    for (int i = 0; i < numDetections; ++i) {
        float confidence = detections[i * 7 + 2];
        if (confidence > 0.5) { // Confidence threshold
            int x1 = static_cast<int>(detections[i * 7 + 3] * frame.cols);
            int y1 = static_cast<int>(detections[i * 7 + 4] * frame.rows);
            int x2 = static_cast<int>(detections[i * 7 + 5] * frame.cols);
            int y2 = static_cast<int>(detections[i * 7 + 6] * frame.rows);
            cv::rectangle(frame, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0), 2);
        }
    }
}

int main() {
    // Open the webcam
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open webcam." << std::endl;
        return -1;
    }

    // Load TensorRT engine
    nvinfer1::IRuntime* runtime = nvinfer1::createInferRuntime(gLogger);
    std::ifstream engineFile("ssd_mobilenet_v2_coco.engine", std::ios::binary);
    if (!engineFile) {
        std::cerr << "Error: Could not open engine file." << std::endl;
        return -1;
    }

    engineFile.seekg(0, engineFile.end);
    size_t engineSize = engineFile.tellg();
    engineFile.seekg(0, engineFile.beg);
    std::vector<char> engineData(engineSize);
    engineFile.read(engineData.data(), engineSize);
    engineFile.close();
    
    nvinfer1::ICudaEngine* engine = runtime->deserializeCudaEngine(engineData.data(), engineSize, nullptr);
    
    if (!engine) {
        std::cerr << "Error: Could not deserialize engine." << std::endl;
        return -1;
    }
    
    nvinfer1::IExecutionContext* context = engine->createExecutionContext();
    if (!context) {
        std::cerr << "Error: Could not create execution context." << std::endl;
        return -1;
    }

    // Allocate memory for input and output
    void* buffers[2];
    cudaMalloc(&buffers[0], INPUT_SIZE * sizeof(float)); 
    cudaMalloc(&buffers[1], OUTPUT_SIZE * sizeof(float)); 

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Could not read frame." << std::endl;
            break;
        }

        // Preprocess the frame for SSD input
        cv::resize(frame, frame, cv::Size(300, 300));
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        frame.convertTo(frame, CV_32FC3, 1.0 / 255);

        // Perform inference using TensorRT
        cudaMemcpy(buffers[0], frame.data, INPUT_SIZE * sizeof(float), cudaMemcpyHostToDevice);
        context->executeV2(buffers);
        float output[OUTPUT_SIZE];
        cudaMemcpy(output, buffers[1], OUTPUT_SIZE * sizeof(float), cudaMemcpyDeviceToHost);

        // Draw detections on the frame
        drawDetections(frame, output, OUTPUT_SIZE / 7);

        // Display the frame
        cv::imshow("Webcam", frame);

        // Exit on 'q' key press
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    // Clean up
    cudaFree(buffers[0]);
    cudaFree(buffers[1]);
    context->destroy();
    engine->destroy();
    runtime->destroy();

    return 0;
}
