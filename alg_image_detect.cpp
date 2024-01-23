// Copyright (c) 2021 by Rockchip Electronics Co., Ltd. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*
Changelog:
    * removed unnecessary code (include ssd.h instead)
    * converted to use ssd_inception_v2 dataset
    * boxes are drawn around detected objects
    * for every detection, an timestamped image is saved
    * images are now resized before model stage

    * converted image acqusition to V4l2
        * ! Some Issues:
            * single iteration run (segmentation fault)
            * Greyscale image in 1080p, 640p bad colour
    * adapted code for the rk3399 board
    * replaced v4l2 with opencv (not multiplanar image)
    * reads all 4 cameras one at a time.
    *
ToDo:
    * adapt code to USB Cam.h
    *
*/

/*-------------------------------------------
                Includes
-------------------------------------------*/
// C standard
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
// STOKNAM
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

// opencv
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
// time and logging
#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <sstream>

#include "rknn_api.h"
// box detection
#include "ssd.h"

// v4l2
#include <assert.h>
#include <getopt.h> /* getopt_long() */
#include <linux/videodev2.h>

// ROS
#include <image_transport/image_transport.h>
#include <ros/ros.h>
#include <ros/time.h>

//#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>

#include <opencv2/imgproc/imgproc.hpp>
//#include <boost/algorithm/string.hpp>
//#include "messages_node/CamStatus.h"
//#include "vdb_release.h"
//#include "usbcam.h"
//#include <cstdlib>

// v4l2
#define CLEAR(x) memset(&(x), 0, sizeof(x))

#define NUM_PLANES 1

enum vb2_memory {
  VB2_MEMORY_UNKNOWN = 0,
  VB2_MEMORY_MMAP = 1,
};

struct buffer {
  void *start[NUM_PLANES];
  size_t length[NUM_PLANES];
};

static char dev_name[20];
static char convertcmd[100];
static int fd = -1;
struct buffer *buffers;
static unsigned int n_buffers;
// static int out_buf;
// static int status;

using namespace std;
using namespace cv;

#define PD_IMAGE_WIDTH 640  /* 算法图像帧宽度 Camera image frame width */
#define PD_IMAGE_HEIGHT 480 /* 算法图像帧高度 Camera image frame height*/

#define ALG_IMAGE_WIDTH 300 /* 算法图像帧宽度 Algorithm image frame width */
#define ALG_IMAGE_HEIGHT 300 /* 算法图像帧高度 Algorithm image frame height*/

int g_as32MediaBufFd[4] = {
    -1};  // 各物理通道的Media Buffer的文件描述符 // The file descriptor of the
          // Media Buffer of each physical channel
int g_s32SocketFd = -1;

int bytes_per_pixel = 3; /* or 1 for GRACYSCALE images */

std::ofstream log_file; /* push variables to be saved to file for analysis*/

const char *g_ssd_path = "/home/firefly/model/ssd_inception_v2_rk180x.rknn";
const char *g_img_path = "/home/firefly/model/road.bmp";

cv::Mat orig_img;
cv::Mat img;

// system parameter file paths to be logged
const char *filepaths[7] = {
    "/sys/devices/virtual/thermal/thermal_zone1/temp",
    "/sys/devices/virtual/thermal/thermal_zone0/temp",
    "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq",
    "/sys/devices/system/cpu/cpu1/cpufreq/cpuinfo_cur_freq",
    "/sys/devices/system/cpu/cpu2/cpufreq/cpuinfo_cur_freq",
    "/sys/devices/system/cpu/cpu3/cpufreq/cpuinfo_cur_freq",
    "/sys/devices/platform/ffbc0000.npu/devfreq/ffbc0000.npu/cur_freq"};

/*-------------------------------------------
            Post Processing Function
-------------------------------------------*/

static void printRKNNTensor(rknn_tensor_attr *attr) {
  printf(
      "index=%d name=%s n_dims=%d dims=[%d %d %d %d] n_elems=%d size=%d fmt=%d "
      "type=%d qnt_type=%d fl=%d zp=%d scale=%f\n",
      attr->index, attr->name, attr->n_dims, attr->dims[3], attr->dims[2],
      attr->dims[1], attr->dims[0], attr->n_elems, attr->size, 0, attr->type,
      attr->qnt_type, attr->fl, attr->zp, attr->scale);
}

static unsigned char *load_model(const char *filename, int *model_size) {
  FILE *fp = fopen(filename, "rb");
  if (fp == nullptr) {
    printf("fopen %s fail!\n", filename);
    return NULL;
  }
  fseek(fp, 0, SEEK_END);
  int model_len = ftell(fp);
  unsigned char *model = (unsigned char *)malloc(model_len);
  fseek(fp, 0, SEEK_SET);
  if (model_len != fread(model, 1, model_len, fp)) {
    printf("fread %s fail!\n", filename);
    free(model);
    return NULL;
  }
  *model_size = model_len;
  if (fp) {
    fclose(fp);
  }
  return model;
}

class InferenceRKNN {
  rknn_context ctx;
  uint32_t model_input_size;
  unsigned char *model;
  rknn_input_output_num io_num;
  rknn_output outputs[2];
  rknn_input inputs[1];

  const char *ssd_path = "/home/firefly/model/ssd_inception_v2_rk180x.rknn";

 public:
  int init() {
    int ret;
    int model_len = 0;

    // Load RKNN Model
    model = load_model(ssd_path, &model_len);

    ret = rknn_init(&ctx, model, model_len, 0);
    if (ret < 0) {
      printf("rknn_init fail! ret=%d\n", ret);
      return -1;
    }

    // Get Model Input Output Info
    ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret != RKNN_SUCC) {
      printf("rknn_query fail! ret=%d\n", ret);
      return -1;
    }

    rknn_tensor_attr input_attrs[io_num.n_input];

    memset(input_attrs, 0, sizeof(input_attrs));
    for (uint32_t i = 0; i < io_num.n_input; i++) {
      input_attrs[i].index = i;
      ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]),
                       sizeof(rknn_tensor_attr));
      if (ret != RKNN_SUCC) {
        printf("rknn_query fail! ret=%d\n", ret);
        return -1;
      }
      // printRKNNTensor(&(input_attrs[i]));
    }

    printf("output tensors:\n");
    rknn_tensor_attr output_attrs[io_num.n_output];
    memset(output_attrs, 0, sizeof(output_attrs));
    for (uint32_t i = 0; i < io_num.n_output; i++) {
      output_attrs[i].index = i;
      ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]),
                       sizeof(rknn_tensor_attr));
      if (ret != RKNN_SUCC) {
        printf("rknn_query fail! ret=%d\n", ret);
        return -1;
      }
      // printRKNNTensor(&(output_attrs[i]));
    }

    // Prepare Input Data
    memset(inputs, 0, sizeof(inputs));
    inputs[0].index = 0;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].fmt = RKNN_TENSOR_NHWC;

    // Prepare Output
    memset(outputs, 0, sizeof(outputs));
    outputs[0].want_float = 1;
    outputs[0].is_prealloc = 0;
    outputs[1].want_float = 1;
    outputs[1].is_prealloc = 0;

    return -1;
  }

  int run(cv::Mat img) {
    int ret;

    inputs[0].buf = img.data;
    inputs[0].size = img.cols * img.rows * img.channels();
    ret = rknn_inputs_set(ctx, io_num.n_input, inputs);
    if (ret < 0) {
      printf("rknn_input_set fail! ret=%d\n", ret);
      return -1;
    }

    // Run
    ret = rknn_run(ctx, nullptr);
    if (ret < 0) {
      printf("rknn_run fail! ret=%d\n", ret);
      return -1;
    }

    // Get RKNN inference output
    // NB: takes the longest time ~30ms... are we waiting for the NPU to
    // complete?
    ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);
    if (ret < 0) {
      printf("rknn_outputs_get fail! ret=%d\n", ret);
      return -1;
    }

    return 0;
  }

  int post_process() {
    detect_result_group_t detect_result_group;
    postProcessSSD((float *)(outputs[0].buf), (float *)(outputs[1].buf),
                   ALG_IMAGE_HEIGHT, ALG_IMAGE_WIDTH, &detect_result_group);

    // Release rknn_outputs
    rknn_outputs_release(ctx, 2, outputs);

    // Draw Objects
    // todo: send to ROS topic

    for (int i = 0; i < detect_result_group.count; i++) {
      detect_result_t *det_result = &(detect_result_group.results[i]);
      printf("%s @ (%d %d %d %d) %f\n", det_result->name, det_result->box.left,
             det_result->box.top, det_result->box.right, det_result->box.bottom,
             det_result->prop);
      int x1 = det_result->box.left;
      int y1 = det_result->box.top;
      int x2 = det_result->box.right;
      int y2 = det_result->box.bottom;

      std::cout << "drawing boxes..."
                << "(" << x1 << ", " << y1 << "), (" << x2 << ", " << y2 << ")"
                << std::endl;
      std::cout << "size: " << sizeof(img) << std::endl;
      rectangle(img, Point(x1, y1), Point(x2, y2), Scalar(255, 0, 0, 255), 3);
      std::cout << "adding text..." << std::endl;
      putText(img, det_result->name, Point(x1, y1 - 12), 1, 2,
              Scalar(0, 255, 0, 255));
    }

    // todo: see how long saving image takes
    if (detect_result_group.count > 0) {
      std::cout << "SAVING IMAGE" << std::endl;

      std::stringstream ss;
      std::string filename_str;
      auto end_milli = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();

      std::cout << "detection_" << end_milli << ".jpg";
      ss << "/media/firefly/0356bd29-ed3d-4b5d-921f-a7a8362599df/"
            "detected_images/detection_"
         << end_milli << ".jpg";
      ss >> filename_str;

      cv::imwrite(filename_str.c_str(), img);
    }

    return 0;
  }

  int free_rknn() {
    // printf("release rknn\n");
    // Release
    if (ctx >= 0) {
      rknn_destroy(ctx);
    }
    // printf("release model\n");
    if (model) {
      free(model);
    }
  }
};

/*-------------------------------------------
            algorithm_node Function
-------------------------------------------*/

/*-------------------------------------------
                Logging Function
-------------------------------------------*/

void open_logger() {
  std::string filename;
  std::stringstream ss;
  std::time_t current_time =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  std::string time_str = std::ctime(&current_time);  // <<std::endl;
  std::replace(time_str.begin(), time_str.end(), ' ',
               '_');  // repalce space with underscore
  std::replace(time_str.begin(), time_str.end(), '\n', '_');

  ss << "/media/firefly/0356bd29-ed3d-4b5d-921f-a7a8362599df/detected_logs/"
     << "log_" << time_str << ".txt";
  ss >> filename;

  log_file.open(filename);
}

void log_memory() {
  std::string str;
  std::string result_str;

  int mem_free = 0;
  int mem_total = 1;
  int mem_percent = 0;

  // iterate past parameter names to get mem values
  std::ifstream ifile("/proc/meminfo");
  ifile >> str;
  ifile >> str;
  mem_total = std::stoi(str);
  ifile >> str;
  ifile >> str;
  ifile >> str;
  mem_free = std::stoi(str);

  // calculate use percentage
  mem_percent = (int)(((float)mem_free / mem_total) * 100);

  log_file << mem_percent << ", " << mem_free << ", " << mem_total << ", ";
  // ss << " mem_percent " << mem_percent << " mem_free " << mem_free << "
  // mem_total " << mem_total;
  // ss >> result_str
  // return result_str;
}

void read_from_file(const char *filename) {
  std::string val;
  // std::cout << filename << "\n";

  std::ifstream ifile;
  ifile.open(filename);
  if (!ifile) {
    std::cout << "file not found: " << filename << "\n";
  }

  ifile >> val;
  ifile.close();
  log_file << val << ", ";
  // std::cout << val << "\n";
}

void save_to_file(Mat img, char *filename) {
  // for testing outputs
  std::string filename_str;
  std::stringstream ss;

  auto start_milli = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();

  std::cout << filename << ".jpg";
  ss << "/media/firefly/0356bd29-ed3d-4b5d-921f-a7a8362599df/detected_images/"
        "detection_"
     << start_milli << ".jpg";
  ss >> filename_str;

  cv::imwrite(filename_str.c_str(), img);
}

/*-------------------------------------------
            Detection Loop
-------------------------------------------*/

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