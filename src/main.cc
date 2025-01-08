#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <opencv2/opencv.hpp>

constexpr int ALG_IMAGE_WIDTH = 300;
constexpr int ALG_IMAGE_HEIGHT = 300;

class CameraProcessor
{
public:
    CameraProcessor(const std::string &port) : camera_port(port), stop_thread(false) {}

    void Start()
    {
        processing_thread = std::thread(&CameraProcessor::ProcessFrames, this);
    }

    void Stop()
    {
        stop_thread = true;
        if (processing_thread.joinable())
        {
            processing_thread.join();
        }
    }

    ~CameraProcessor()
    {
        Stop();
        if (cap.isOpened())
        {
            cap.release();
        }
    }

private:
    std::string camera_port;
    cv::VideoCapture cap;
    std::thread processing_thread;
    bool stop_thread;

    void ProcessFrames()
    {
        cap.open(camera_port);
        if (!cap.isOpened())
        {
            std::cerr << "No video stream detected on " << camera_port << std::endl;
            return;
        }

        std::cout << "\n============================================" << std::endl;
        std::cout << "Found Camera: " << camera_port << std::endl;
        std::cout << "============================================" << std::endl;

        cv::Mat orig_img, img;

        while (!stop_thread)
        {
            auto start_milli = std::chrono::system_clock::now();

            cap >> orig_img;
            if (orig_img.empty())
            {
                std::cerr << "ERROR! blank frame grabbed from " << camera_port << std::endl;
                continue;
            }

            auto image_read_milli = std::chrono::system_clock::now();
            cv::resize(orig_img, img, cv::Size(ALG_IMAGE_WIDTH, ALG_IMAGE_HEIGHT), 0, 0, cv::INTER_LINEAR);
            auto image_resize_milli = std::chrono::system_clock::now();

            // Placeholder for main algorithm
            auto alg_main_milli = std::chrono::system_clock::now();

            // Placeholder for post-processing
            auto end_milli = std::chrono::system_clock::now();

            std::cout << "image_read_milli: " << std::chrono::duration_cast<std::chrono::milliseconds>(image_read_milli - start_milli).count() << " ms\n";
            std::cout << "image_resize_milli: " << std::chrono::duration_cast<std::chrono::milliseconds>(image_resize_milli - image_read_milli).count() << " ms\n";
            std::cout << "alg_main_milli: " << std::chrono::duration_cast<std::chrono::milliseconds>(alg_main_milli - image_resize_milli).count() << " ms\n";
            std::cout << "total: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_milli - start_milli).count() << " ms\n";

            auto real_end_milli = std::chrono::system_clock::now();
            float fps = 1000.0f / std::chrono::duration_cast<std::chrono::milliseconds>(real_end_milli - start_milli).count();
            std::cout << "fps: " << fps << "\n\n";

            orig_img.release();
            img.release();
        }
    }
};

int main()
{
    std::vector<std::string> camera_ports = {"/dev/video0", "/dev/video4", "/dev/video10", "/dev/video14"};
    std::vector<CameraProcessor> cameras;

    for (const auto &port : camera_ports)
    {
        cameras.emplace_back(port);
    }

    for (auto &camera : cameras)
    {
        camera.Start();
    }

    std::cout << "Press any key to stop..." << std::endl;
    std::cin.get();

    for (auto &camera : cameras)
    {
        camera.Stop();
    }

    std::cout << "All cameras stopped." << std::endl;
    return 0;
}
