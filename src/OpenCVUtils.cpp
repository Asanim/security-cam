#include "OpenCVUtils.h"

void OpenCVUtils::saveImageToFile(const cv::Mat& img, const std::string& filename) {
    std::string filename_str;
    std::stringstream ss;

    auto start_milli = std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();

    std::cout << filename << ".jpg";
    ss << "/media/firefly/0356bd29-ed3d-4b5d-921f-a7a8362599df/detected_images/"
          "detection_"
       << start_milli << ".jpg";
    ss >> filename_str;

    cv::imwrite(filename_str.c_str(), img);
}

