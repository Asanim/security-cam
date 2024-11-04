#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>

constexpr int ALG_IMAGE_WIDTH = 300;
constexpr int ALG_IMAGE_HEIGHT = 300;

int main() {
    // Define camera ports
    std::string camera_ports[4] = {"/dev/video0", "/dev/video4", "/dev/video10", "/dev/video14"};
    cv::VideoCapture cap[4];  // Declare an array for multiple VideoCapture instances

    // Open each camera stream
    for (int i = 0; i < 4; ++i) {
        cap[i].open(camera_ports[i]);
        if (!cap[i].isOpened()) {
            std::cout << "No video stream detected on " << camera_ports[i] << std::endl;
        } else {
            std::cout << "\n============================================" << std::endl;
            std::cout << "Found Camera: " << camera_ports[i] << std::endl;
            std::cout << "============================================" << std::endl;
        }
    }

    cv::Mat orig_img, img;
    while (true) {
        // Capture and process frames for each camera
        for (int i = 0; i < 4; ++i) {
            if (!cap[i].isOpened()) continue;

            auto start_milli = std::chrono::system_clock::now();

            // Read frame from camera
            cap[i] >> orig_img;
            if (orig_img.empty()) {
                std::cerr << "ERROR! blank frame grabbed from " << camera_ports[i] << std::endl;
                continue;
            }

            auto image_read_milli = std::chrono::system_clock::now();

            // Resize image
            cv::resize(orig_img, img, cv::Size(ALG_IMAGE_WIDTH, ALG_IMAGE_HEIGHT), 0, 0, cv::INTER_LINEAR);
            auto image_resize_milli = std::chrono::system_clock::now();

            // Placeholder for main algorithm (e.g., inference)
            // inference.run(img); // Replace with actual inference logic if needed
            auto alg_main_milli = std::chrono::system_clock::now();

            // Placeholder for post-processing
            // inference.post_process(); // Replace with actual post-processing logic if needed
            auto end_milli = std::chrono::system_clock::now();

            // Print timing for each processing stage
            std::cout << "image_read_milli: " << std::chrono::duration_cast<std::chrono::milliseconds>(image_read_milli - start_milli).count() << " ms\n";
            std::cout << "image_resize_milli: " << std::chrono::duration_cast<std::chrono::milliseconds>(image_resize_milli - image_read_milli).count() << " ms\n";
            std::cout << "alg_main_milli: " << std::chrono::duration_cast<std::chrono::milliseconds>(alg_main_milli - image_resize_milli).count() << " ms\n";
            std::cout << "total: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_milli - start_milli).count() << " ms\n";

            // Calculate FPS
            auto real_end_milli = std::chrono::system_clock::now();
            float fps = 1000.0f / std::chrono::duration_cast<std::chrono::milliseconds>(real_end_milli - start_milli).count();
            std::cout << "fps: " << fps << "\n\n";
        }

        // Release images
        orig_img.release();
        img.release();

        // Delay for a short while, for demonstration
        if (cv::waitKey(1) >= 0) break;  // Press any key to exit
    }

    // Release cameras
    for (int i = 0; i < 4; ++i) {
        if (cap[i].isOpened()) {
            cap[i].release();
        }
    }

    std::cout << "Release complete" << std::endl;
    return 0;
}
