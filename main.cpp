#include <opencv2/opencv.hpp>
#include <NvInfer.h>
#include <NvInferRuntime.h>
#include <cuda_runtime_api.h>
#include <iostream>

int main() {
    // Open the webcam
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open webcam." << std::endl;
        return -1;
    }

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Could not read frame." << std::endl;
            break;
        }

        // Display the frame
        cv::imshow("Webcam", frame);

        // Exit on 'q' key press
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    return 0;
}
