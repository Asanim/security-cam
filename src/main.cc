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
 *  \file main.cc
 */

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <opencv2/opencv.hpp>

#include "camera-handler.h"
#include "web-streamer.h"

constexpr int ALG_IMAGE_WIDTH = 300;
constexpr int ALG_IMAGE_HEIGHT = 300;

int main()
{
    std::vector<std::string> camera_ports = {"/dev/video0", "/dev/video4", "/dev/video10", "/dev/video14"};
    std::vector<CameraHandler> cameras;
    WebStreamer webStreamer;

    for (const auto &port : camera_ports)
    {
        cameras.emplace_back(port);
    }

    for (auto &camera : cameras)
    {
        camera.Start();
    }

    webStreamer.Start();

    std::cout << "Press any key to stop..." << std::endl;
    std::cin.get();

    for (auto &camera : cameras)
    {
        camera.Stop();
    }

    webStreamer.Stop();

    std::cout << "All cameras stopped." << std::endl;
    return 0;
}
