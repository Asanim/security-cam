#pragma once

#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>
#include <string>
#include "yolov7.h"

class DetectionPipeline {
public:
    DetectionPipeline(const std::string& engine_path, const std::string& video_path);
    void start();
    void stop();

private:
    void process();

    std::string engine_path;
    std::string video_path;
    std::thread processing_thread;
    bool running;
};
