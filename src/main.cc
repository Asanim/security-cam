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

int main() {
  std::cout << "main.cc" << std::endl;
  return 0;
}


// int detection_stage()
int main(int argc, char **argv) {
  uint32_t model_input_size;
  std::string camera_ports[4] = {"/dev/video0", "/dev/video4", "/dev/video10",
                                 "/dev/video14"};
  unsigned char *RKNN_model;

  InferenceRKNN inference;

  std::string video_output = "/dev/video";

  cv::VideoCapture cap[4];  // Declaring an object to capture stream of frames
  // from default camera//

  std::to_string video_name = video_output + std::to_string(video_index);
  cap[i].open(video_name);

  if (!cap[i].isOpened()) {
    cout << "No video stream detected" << video_name << endl;
  } else {
    cout << endl;
    cout << "============================================ " << endl;
    cout << "Found Camera: " << video_name << endl;
    cout << "============================================ " << endl;
    cout << endl;
  }

  inference.init();

  while (1) {
    auto start_milli = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();

    // check if we succeeded
    if (orig_img.empty()) {
      cerr << "ERROR! blank frame grabbed\n";
      break;
    }
    auto image_read_milli =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();

    cv::resize(orig_img, img, cv::Size(ALG_IMAGE_WIDTH, ALG_IMAGE_HEIGHT),
               (0, 0), (0, 0), cv::INTER_LINEAR);
    auto image_resize_milli =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();

    inference.run(img);

    auto alg_main_milli =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();

    inference.post_process();

    auto end_milli = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();

    // calculation just after!
    printf("image_read_milli: %ld", (image_read_milli - start_milli));
    printf("image_resize_milli: %ld", (image_resize_milli - image_read_milli));
    printf("alg_main_milli: %ld", (alg_main_milli - image_resize_milli));
    printf("alg_post_milli: %ld", (end_milli - alg_main_milli));
    printf("total : %ld", (end_milli - start_milli));

    auto real_end_milli =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();
    float fps = 0.0;
    if ((real_end_milli - start_milli) > 0) {
      fps = 1000 / ((float)(real_end_milli - start_milli));
      std::cout << "fps" << fps << "\n\n";
    }

    orig_img.release();
    img.release();
  }

  inference.free_rknn();

  printf("release camera buffer\n");
  // release
  /*
  for (i = 0; i < s32ChnNum; i++)
  {
      //check if we have a camera first otherwise segmentation fault
      if (MAP_FAILED != apvBuf[i])
      {
          munmap(apvBuf[i], u32BufLen);
      }
  }
  */
  printf("release log buffer\n");
  log_file.close();

  /*
   *   TODO: write to analog display out
   */
  return -1;
}