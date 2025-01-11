/* 
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 *  
 *  \file camera-handler.cc
 */

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>
#include "camera-handler.h"
CameraProcessor::CameraProcessor(const std::string &port) : camera_port(port), stop_thread(false) {}

void CameraProcessor::Start()
{
    processing_thread = std::thread(&CameraProcessor::ProcessFrames, this);
}

void CameraProcessor::Stop()
{
    stop_thread = true;
    if (processing_thread.joinable())
    {
        processing_thread.join();
    }
}

CameraProcessor::~CameraProcessor()
{
    Stop();
    if (cap.isOpened())
    {
        cap.release();
    }
}

void CameraProcessor::ProcessFrames()
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