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
#include "yolov7.h"

class Logger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) noexcept override {
        // suppress info-level messages
        if (severity <= Severity::kWARNING) {
            std::cout << msg << std::endl;
        }
    }
} gLogger;

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <engine_path> <video_path>" << std::endl;
        return -1;
    }

    std::string engine_path = argv[1];
    std::string video_path = argv[2];

    Yolov7 yolov7(engine_path);

    cv::VideoCapture capture(video_path);
    if (!capture.isOpened()) {
        std::cerr << "Error: Could not open video." << std::endl;
        return -1;
    }

    cv::Size size = cv::Size(capture.get(cv::CAP_PROP_FRAME_WIDTH), capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    cv::VideoWriter writer(video_path + ".detect.mp4", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, size, true);

    cv::Mat frame;
    std::vector<cv::Mat> framev;
    std::vector<std::vector<std::vector<float>>> nmsresults;
    int total_frame_count = capture.get(cv::CAP_PROP_FRAME_COUNT);
    int i = 0;

    while (capture.read(frame)) {
        framev.push_back(frame);
        yolov7.preProcess(framev);
        yolov7.infer();
        nmsresults = yolov7.PostProcess();
        Yolov7::DrawBoxesonGraph(frame, nmsresults[0]);
        writer.write(frame);
        framev.clear();
        i++;
        std::cout << "\r" << i << " / " << total_frame_count;
        std::cout.flush();
    }

    capture.release();
    std::cout << "Done..." << std::endl;

    return 0;
}
