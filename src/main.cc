#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <opencv2/opencv.hpp>

constexpr int ALG_IMAGE_WIDTH = 300;
constexpr int ALG_IMAGE_HEIGHT = 300;

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
