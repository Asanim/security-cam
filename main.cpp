#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "detection_pipeline.h"
#include "rest_api_server.h"

class Logger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) noexcept override {
        // suppress info-level messages
        if (severity <= Severity::kWARNING) {
            std::cout << msg << std::endl;
        }
    }
} gLogger;

std::vector<std::string> enumerateCameras() {
    std::vector<std::string> cameras;
    for (int i = 0; i < 10; ++i) {
        cv::VideoCapture cap(i);
        if (cap.isOpened()) {
            cameras.push_back(std::to_string(i));
            cap.release();
        }
    }
    return cameras;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <engine_path> [<video_path1> <video_path2> ...]" << std::endl;
        return -1;
    }

    std::string engine_path = argv[1];
    std::vector<std::string> video_paths;

    for (int i = 2; i < argc; ++i) {
        video_paths.push_back(argv[i]);
    }

    if (video_paths.empty()) {
        video_paths = enumerateCameras();
    }

    std::vector<std::unique_ptr<DetectionPipeline>> pipelines;
    for (const auto& video_path : video_paths) {
        pipelines.push_back(std::make_unique<DetectionPipeline>(engine_path, video_path));
    }

    for (auto& pipeline : pipelines) {
        pipeline->start();
    }

    Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(9080));
    RestApiServer apiServer(addr);

    apiServer.init();
    apiServer.start();

    for (auto& pipeline : pipelines) {
        pipeline->stop();
    }

    return 0;
}
