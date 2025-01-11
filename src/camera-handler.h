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
 *  \file camera-handler.h
 */

 #ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include <string>
#include <thread>
#include <opencv2/opencv.hpp>

class CameraProcessor
{
public:
    CameraProcessor(const std::string &port);
    void Start();
    void Stop();
    ~CameraProcessor();

private:
    std::string camera_port;
    cv::VideoCapture cap;
    std::thread processing_thread;
    bool stop_thread;

    void ProcessFrames();
};

#endif // CAMERA_HANDLER_H
