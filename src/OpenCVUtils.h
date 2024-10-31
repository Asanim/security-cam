#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <sstream>
#include <chrono>

class OpenCVUtils {
public:
    static void saveImageToFile(const cv::Mat& img, const std::string& filename);
};

