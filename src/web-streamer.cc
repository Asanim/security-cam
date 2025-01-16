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
 *  \file web-streamer.cc
 */

#include "web-streamer.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <mutex>
#include <mjpeg_server.h>

WebStreamer::WebStreamer() : running(false) {}

WebStreamer::~WebStreamer()
{
    Stop();
}

void WebStreamer::Start()
{
    running = true;
    stream_thread = std::thread(&WebStreamer::StreamLoop, this);
    std::cout << "Web stream started." << std::endl;
}

void WebStreamer::Stop()
{
    running = false;
    if (stream_thread.joinable())
    {
        stream_thread.join();
    }
    std::cout << "Web stream stopped." << std::endl;
}

void WebStreamer::UpdateFrame(const cv::Mat &frame)
{
    std::lock_guard<std::mutex> lock(frame_mutex);
    frame.copyTo(current_frame);
}

void WebStreamer::StreamLoop()
{
    mjpeg::Server server(8080);
    while (running)
    {
        cv::Mat frame;
        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            if (current_frame.empty())
            {
                continue;
            }
            current_frame.copyTo(frame);
        }

        if (!frame.empty())
        {
            std::vector<uchar> buf;
            cv::imencode(".jpg", frame, buf);
            server.write(buf.data(), buf.size());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}
