#include "detection_pipeline.h"
#include <iostream>

DetectionPipeline::DetectionPipeline(const std::string& engine_path, const std::string& video_path)
    : engine_path(engine_path), video_path(video_path), running(false) {}

void DetectionPipeline::start() {
    running = true;
    processing_thread = std::thread(&DetectionPipeline::process, this);
}

void DetectionPipeline::stop() {
    running = false;
    if (processing_thread.joinable()) {
        processing_thread.join();
    }
}

void DetectionPipeline::process() {
    Yolov7 yolov7(engine_path);

    cv::VideoCapture capture(video_path);
    if (!capture.isOpened()) {
        std::cerr << "Error: Could not open video." << std::endl;
        return;
    }

    cv::Size size = cv::Size(capture.get(cv::CAP_PROP_FRAME_WIDTH), capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    cv::VideoWriter writer(video_path + ".detect.mp4", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, size, true);

    cv::Mat frame;
    std::vector<cv::Mat> framev;
    std::vector<std::vector<std::vector<float>>> nmsresults;
    int total_frame_count = capture.get(cv::CAP_PROP_FRAME_COUNT);
    int i = 0;

    while (running && capture.read(frame)) {
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
}
