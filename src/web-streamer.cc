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
