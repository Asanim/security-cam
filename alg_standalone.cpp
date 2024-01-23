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
    * 
ToDo:
    * adapt code to USB Cam.h
    * 
*/


/*-------------------------------------------
                Includes
-------------------------------------------*/
//C standard
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
//STOKNAM 
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/mman.h> 
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/un.h>

#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

//opencv
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
//time and logging
#include <fstream>
#include "rknn_api.h"
#include <chrono>
#include <ctime>
#include <sstream>

#include <algorithm>
//box detection
#include "ssd.h"

//v4l2
#include <assert.h>
#include <getopt.h>             /* getopt_long() */
#include <linux/videodev2.h>

//v4l2
#define CLEAR(x) memset(&(x), 0, sizeof(x))

#define NUM_PLANES 1

enum vb2_memory {
	VB2_MEMORY_UNKNOWN      = 0,
	VB2_MEMORY_MMAP         = 1,
};

struct buffer {
        void   *start[NUM_PLANES];
        size_t  length[NUM_PLANES];
};

static char dev_name[20];
static char convertcmd[100];
static int fd = -1;
struct buffer *buffers;
static unsigned int n_buffers;
//static int out_buf;
//static int status;


using namespace std;
using namespace cv;


#define PD_IMAGE_WIDTH      640             /* 算法图像帧宽度 Camera image frame width */
#define PD_IMAGE_HEIGHT     480              /* 算法图像帧高度 Camera image frame height*/

#define ALG_IMAGE_WIDTH      300             /* 算法图像帧宽度 Algorithm image frame width */
#define ALG_IMAGE_HEIGHT     300             /* 算法图像帧高度 Algorithm image frame height*/


int g_as32MediaBufFd[4] = {-1};     // 各物理通道的Media Buffer的文件描述符 // The file descriptor of the Media Buffer of each physical channel 
int g_s32SocketFd = -1;

int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */

std::ofstream log_file; /* push variables to be saved to file for analysis*/

const char *g_ssd_path = "/home/firefly/model/ssd_inception_v2_rk180x.rknn";
const char *g_img_path = "./road.bmp";
cv::Mat orig_img;
cv::Mat img;
cv::Mat raw_img;


//system parameter file paths to be logged
const char *filepaths[7] = {
"/sys/devices/virtual/thermal/thermal_zone1/temp",
"/sys/devices/virtual/thermal/thermal_zone0/temp",
"/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq",
"/sys/devices/system/cpu/cpu1/cpufreq/cpuinfo_cur_freq",
"/sys/devices/system/cpu/cpu2/cpufreq/cpuinfo_cur_freq",
"/sys/devices/system/cpu/cpu3/cpufreq/cpuinfo_cur_freq",
"/sys/devices/platform/ffbc0000.npu/devfreq/ffbc0000.npu/cur_freq"
};

/*-------------------------------------------
            Post Processing Function
-------------------------------------------*/

static void printRKNNTensor(rknn_tensor_attr *attr)
{
    printf("index=%d name=%s n_dims=%d dims=[%d %d %d %d] n_elems=%d size=%d fmt=%d type=%d qnt_type=%d fl=%d zp=%d scale=%f\n",
           attr->index, attr->name, attr->n_dims, attr->dims[3], attr->dims[2], attr->dims[1], attr->dims[0],
           attr->n_elems, attr->size, 0, attr->type, attr->qnt_type, attr->fl, attr->zp, attr->scale);
}

static unsigned char *load_model(const char *filename, int *model_size)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == nullptr)
    {
        printf("fopen %s fail!\n", filename);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    unsigned char *model = (unsigned char *)malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if (model_len != fread(model, 1, model_len, fp))
    {
        printf("fread %s fail!\n", filename);
        free(model);
        return NULL;
    }
    *model_size = model_len;
    if (fp)
    {
        fclose(fp);
    }
    return model;
}

/*-------------------------------------------
            algorithm_node Function
-------------------------------------------*/
int algorithm_node_init(rknn_context *ctx, uint32_t *model_input_size, unsigned char *model, int *model_ninput, int *model_noutput)
{
    int ret;
    int model_len = 0;

    // Load RKNN Model
    model = load_model(g_ssd_path, &model_len);

    ret = rknn_init(ctx, model, model_len, 0);
    if (ret < 0)
    {
        printf("rknn_init fail! ret=%d\n", ret);
        return -1;
    }

    // Get Model Input Output Info
    rknn_input_output_num io_num;
    ret = rknn_query(*ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret != RKNN_SUCC)
    {
        printf("rknn_query fail! ret=%d\n", ret);
        return -1;
    }
    printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);

    printf("input tensors:\n");
    rknn_tensor_attr input_attrs[io_num.n_input];
    //std::cout << "io_num.n_input: " << io_num.n_input;

    memset(input_attrs, 0, sizeof(input_attrs));
    for (uint32_t i = 0; i < io_num.n_input; i++)
    {
        input_attrs[i].index = i;
        ret = rknn_query(*ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret != RKNN_SUCC)
        {
            printf("rknn_query fail! ret=%d\n", ret);
            return -1;
        }
        printf("n_input: %d \n", i);
        printRKNNTensor(&(input_attrs[i]));
    }

    printf("output tensors:\n");
    rknn_tensor_attr output_attrs[io_num.n_output];
    memset(output_attrs, 0, sizeof(output_attrs));
    for (uint32_t i = 0; i < io_num.n_output; i++)
    {
        output_attrs[i].index = i;
        ret = rknn_query(*ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret != RKNN_SUCC)
        {
            printf("rknn_query fail! ret=%d\n", ret);
            return -1;
        }
        printRKNNTensor(&(output_attrs[i]));
    }
    *model_input_size = input_attrs[0].size;
    *model_ninput = io_num.n_input;
    *model_noutput = io_num.n_output;
    std::cout << "ALG_INIT n_input: " << io_num.n_input << " n_output: " << io_num.n_output << " input_attrs size: " << input_attrs[0].size << " \n";
    return -1;
}




int algorithm_node_run(rknn_context *ctx, unsigned char *input_data, uint32_t *model_input_size, int *model_ninput) 
{
    int ret;
    printf("rknn_run inputs pre\n");

    // Set Input Data
    rknn_input inputs[1];
    memset(inputs, 0, sizeof(inputs));
    inputs[0].index = 0;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].size = img.cols*img.rows*img.channels();; // inputs[0].size = input_attrs[0].size; static
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    inputs[0].buf = img.data;

    printf("rknn_run inputs set\n");

    std::cout << " ALG_RUN in: " << *model_ninput << " size: " << *model_input_size << " CTX: " << *ctx << "img len: " << strlen((char*)input_data) << "\n";

    ret = rknn_inputs_set(*ctx, *model_ninput, inputs);
    if (ret < 0)
    {
        printf("rknn_input_set fail! ret=%d\n", ret);
        return -1;
    }

    // Run
    printf("rknn_run\n");
    ret = rknn_run(*ctx, nullptr);
    if (ret < 0)
    {
        printf("rknn_run fail! ret=%d\n", ret);
        return -1;
    }
    printf("rknn_run complete!\n");

    return 0;
}


int algorithm_node_post(rknn_context *ctx, unsigned char *model, int *model_noutput, Mat orig_img)
{
    
    std::cout << "entered post processing\n";
    int ret;
    // Get Output
    rknn_output outputs[2];
    memset(outputs, 0, sizeof(outputs));
    outputs[0].want_float = 1;
    outputs[0].is_prealloc = 0;
    outputs[1].want_float = 1;
    outputs[1].is_prealloc = 0;

    std::cout << "getting outputs\n";
    ret = rknn_outputs_get(*ctx, *model_noutput, outputs, NULL);
    if (ret < 0)
    {
        printf("rknn_outputs_get fail! ret=%d\n", ret);
        return -1;
    }

    
    detect_result_group_t detect_result_group;
    postProcessSSD((float *)(outputs[0].buf), (float *)(outputs[1].buf), ALG_IMAGE_HEIGHT, ALG_IMAGE_WIDTH, &detect_result_group);
    // Release rknn_outputs
    rknn_outputs_release(*ctx, 1, outputs);

    std::cout << "detected: " << detect_result_group.count << std::endl;

    // Draw Objects
    for (int i = 0; i < detect_result_group.count; i++) {
        detect_result_t *det_result = &(detect_result_group.results[i]);
        printf("%s @ (%d %d %d %d) %f\n",
                det_result->name,
                det_result->box.left, det_result->box.top, det_result->box.right, det_result->box.bottom,
                det_result->prop);
        int x1 = det_result->box.left;
        int y1 = det_result->box.top;
        int x2 = det_result->box.right;
        int y2 = det_result->box.bottom;

        std::cout << "drawing boxes..." << "(" << x1 << ", " << y1  << "), (" << x2 << ", " << y2  << ")" << std::endl;
        std::cout << "size: " << sizeof(img) << std::endl;
        rectangle(img, Point(x1, y1), Point(x2, y2), Scalar(255, 0, 0, 255), 3);
        std::cout << "adding text..." << std::endl;
        putText(img, det_result->name, Point(x1, y1 - 12), 1, 2, Scalar(0, 255, 0, 255));
    }


    if (detect_result_group.count > 0) {
        std::cout << "SAVING IMAGE" << std::endl;

        std::stringstream ss;
        std::string filename_str;
        auto end_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  
        std::cout << "detection_" << end_milli << ".jpg";
        ss << "detection_" << end_milli << ".jpg";
        ss >> filename_str;

        cv::imwrite(filename_str.c_str(), img);
    }
    
    return 0;
}





/*-------------------------------------------
                Logging Function
-------------------------------------------*/

void open_logger () {
	std::string filename;
	std::stringstream ss;
	std::time_t current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
 	
	std::string time_str = std::ctime(&current_time);// <<std::endl;
  	std::replace( time_str.begin(), time_str.end(), ' ', '_'); // repalce space with underscore
	std::replace( time_str.begin(), time_str.end(), '\n', '_');


	ss << "/sdcard/" << "log_" << time_str << ".txt";
	ss >> filename;

	log_file.open(filename);
}

void log_memory () {
	std::string str;
    std::string result_str;

	int mem_free = 0;
	int mem_total = 1;
	int mem_percent = 0;

	//iterate past parameter names to get mem values
    std::ifstream ifile("/proc/meminfo");
	ifile >> str;
	ifile >> str;
	mem_total = std::stoi(str);
	ifile >> str;
	ifile >> str;
	ifile >> str;
	mem_free = std::stoi(str);

	//calculate use percentage 
	mem_percent = (int) (((float) mem_free/mem_total)*100);

    	log_file << mem_percent << ", " << mem_free << ", " << mem_total <<  ", ";
	//ss << " mem_percent " << mem_percent << " mem_free " << mem_free << " mem_total " << mem_total;
    //ss >> result_str
    //return result_str;
}

void read_from_file (const char *filename) {
	std::string val;
    //std::cout << filename << "\n";

    std::ifstream ifile;
    ifile.open (filename);
	if (!ifile) {
		std::cout << "file not found: " << filename << "\n";
	}

	ifile >> val;
    ifile.close();
	log_file << val << ", ";
    //std::cout << val << "\n";
}





/*-------------------------------------------
            v4l2
-------------------------------------------*/
static int xioctl(int fh, int request, void *arg)
{
        int r;
	int i = 0;
        do {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
}

static void stop_capturing(void)
{
        enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type)) {
		fprintf(stderr, "VIDIOC_STREAMOFF: error - %d\n", errno);
		exit(EXIT_FAILURE);
	}
}

static void start_capturing(void)
{
        unsigned int i;
        enum v4l2_buf_type type;

	for (i = 0; i < n_buffers; ++i) {
		struct v4l2_buffer buf;
		struct v4l2_plane mplanes[1];

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = VB2_MEMORY_MMAP;
		buf.index = i;
		buf.m.planes	= mplanes;
		buf.length	= NUM_PLANES;

		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
			fprintf(stderr, "VIDIOC_QBUF error: %d\n", errno);
			exit(EXIT_FAILURE);
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	if (-1 == xioctl(fd, VIDIOC_STREAMON, &type)) {
		fprintf(stderr, "VIDIOC_STREAMON error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
}

static void uninit_device(void)
{
        unsigned int i, j;
	for (i = 0; i < n_buffers; ++i)
		for (j = 0; j < NUM_PLANES; j++)
			munmap(buffers[i].start[j], buffers[i].length[j]);
        free(buffers);
}

static void init_mmap(void)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        req.memory = VB2_MEMORY_MMAP;

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        	fprintf(stderr, "VIDIOC_REQBUFS: error - %d\n", errno);
		exit(EXIT_FAILURE);
	}

        if (req.count < 2) {
                fprintf(stderr, "Insufficient buffer memory on %s\n",
                         dev_name);
                exit(EXIT_FAILURE);
        }

        buffers = (buffer*) calloc(req.count, sizeof(*buffers));

        if (!buffers) {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                struct v4l2_buffer buf;
		struct v4l2_plane mplanes[NUM_PLANES];

                CLEAR(buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
                buf.memory      = VB2_MEMORY_MMAP;
                buf.index       = n_buffers;
		buf.m.planes	= mplanes;
		buf.length	= NUM_PLANES;
 
		if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf)) {
        		fprintf(stderr, "VIDIOC_QUERYBUF: error - %d\n", errno);
			exit(EXIT_FAILURE);
		}

		for(int j=0; j<NUM_PLANES; j++) {
                	buffers[n_buffers].length[j] = buf.m.planes[j].length;
			buffers[n_buffers].start[j] = mmap(NULL, buf.m.planes[j].length,
						PROT_READ | PROT_WRITE, /* recommended */
						MAP_SHARED,             /* recommended */
						fd, buf.m.planes[j].m.mem_offset);
                	if (MAP_FAILED == buffers[n_buffers].start[j]) {
				fprintf(stderr, "mmap: error - %d\n", errno);
				exit(EXIT_FAILURE);
			}
		}
        }
}


static void init_device(void)
{
        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;
	struct v4l2_streamparm parm;
        unsigned int min;

        if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
        	fprintf(stderr, "VIDIOC_QUERYCAP: error - %d\n", errno);
		exit(EXIT_FAILURE);
	}

  if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
      fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  } else if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
      fmt.type = V4L2_CAP_VIDEO_CAPTURE;
  } else {
    fprintf(stderr, "%s is no video capture device\n",
              dev_name);

    exit(EXIT_FAILURE);
  }

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf(stderr, "%s does not support streaming i/o\n",
			 dev_name);
		exit(EXIT_FAILURE);
	}

        CLEAR(fmt);
 
	fmt.fmt.pix_mp.width       = PD_IMAGE_WIDTH; //replace
	fmt.fmt.pix_mp.height      = PD_IMAGE_HEIGHT; //replace
	fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12; //V4L2_PIX_FMT_SBGGR8; //replace
	//fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_SBGGR10; //replace
	fmt.fmt.pix_mp.field       = V4L2_FIELD_ANY;
	fmt.fmt.pix_mp.num_planes  = 1;
	fmt.fmt.pix_mp.plane_fmt[0].bytesperline  = PD_IMAGE_WIDTH;
	fmt.fmt.pix_mp.plane_fmt[0].sizeimage  = PD_IMAGE_WIDTH * PD_IMAGE_HEIGHT;

	if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) {
		fprintf(stderr, "VIDIOC_S_FMT: error: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	init_mmap();
}

static void close_device(void)
{
        if (-1 == close(fd)) {
                fprintf(stderr, "Failed to close device\n");
		exit(EXIT_FAILURE);
	}

        fd = -1;
}

static void open_device(void)
{
        fd = open(dev_name, O_RDWR /* required *//* | O_NONBLOCK*/, 0);

        if (fd == -1) {
                fprintf(stderr, "Cannot open '%s': %d\n", dev_name, errno);
                exit(EXIT_FAILURE);
        }
}


/*-------------------------------------------
            Main Loop
-------------------------------------------*/

int main(int argc, char **argv)
{
    std::cout << "Have " << argc << " arguments:" << std::endl;
    for (int i = 0; i < argc; ++i) {
        std::cout << i << ": " << argv[i] << std::endl;
    }


    //image acqusision (STONKAM)
    //void *apvBuf[4] = {NULL};
    //uint32 u32BufLen = PD_IMAGE_WIDTH*PD_IMAGE_HEIGHT*3;
    uint32_t model_input_size;

    //rknn model
    rknn_context ctx;
    unsigned char *RKNN_model;
    int model_ninput; //may be related to model input size
    int model_noutput;
    strcpy(dev_name, "/dev/video0");
    
    //open_logger();

    //open device for multiplanar video 
	  //open_device();
    //init_device();
    //start_capturing();

    //opencv camera - for testing only    
    cv::VideoCapture cap(0);//Declaring an object to capture stream of frames from default camera//
    if (!cap.isOpened()){ //This section prompt an error message if no video stream is found//
      cout << "No video stream detected" << endl;
      return-1;
    }

    algorithm_node_init(&ctx, &model_input_size, RKNN_model, &model_ninput, &model_noutput);

    //avoid segfault (DEBUG only)
    while (1)
    {
        /*********
        * Image Acquisition
        **********/
       /*
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        tv.tv_sec = 4;
        tv.tv_usec = 0;

        r = select(fd + 1, &fds, NULL, NULL, &tv);

        if (-1 == r) {
            if (EINTR == errno)
                //continue;
            fprintf(stderr, "select error\n");
            exit(EXIT_FAILURE);
        }

        if (0 == r) {
            fprintf(stderr, "select timeout\n");
            stop_capturing();
            exit(EXIT_FAILURE);
        }

        struct v4l2_buffer buf;
        struct v4l2_plane mplanes[1];
        unsigned int i;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory = VB2_MEMORY_MMAP;
        buf.m.planes    = mplanes;
        buf.length      = NUM_PLANES;

        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
            case EAGAIN:
                return -1;
            default:
                fprintf(stderr, "VIDIOC_DQBUF: error - %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }

        assert(buf.index < n_buffers);
        printf("plane bytesused (size): %u\n", buf.m.planes->bytesused);
        printf("height: %u\n", (buf.m.planes->bytesused/PD_IMAGE_WIDTH));
        printf("width: %u\n", PD_IMAGE_WIDTH);

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
            fprintf(stderr, "VIDIOC_QBUF error: %d\n", errno);
            exit(EXIT_FAILURE);
        }
        */
        cap >> orig_img;

        /*********
        * resize
        **********/
        auto start_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        printf("resize - copying into Mat array \n");
        //NV12
        //CV_8UC3   cv::COLOR_YUV420p2BGR PD_IMAGE_HEIGHT  
        // (int) (PD_IMAGE_HEIGHT*3.0/2.0)
        //raw_img = cv::Mat( buf.m.planes->bytesused/PD_IMAGE_WIDTH, PD_IMAGE_WIDTH, CV_8UC1, buffers[buf.index].start[0]); //
        
        //cv::cvtColor(raw_img, orig_img, COLOR_YUV2RGB_YV12);
        
        //printf("img size: %d \n", sizeof(orig_img.data));
        //cv::cvtColor(orig_img, gray, cv::COLOR_YUV420p2BGR);
        //cv::cvtColor(orig_img, gray, cv::COLOR_YUV2GRAY_YVYU);
        //printf("resize - writing to file: orig_img \n");
        //cv::imwrite("orig_img.jpg", orig_img);


        //printf("resize - writing to file: BGR_img \n");
        //cv::imwrite("BGR_img.png", BGR_img);

        printf("resize - clone \n");
        img = orig_img.clone();
        if(!orig_img.data) {
            //printf("cv::imread %s fail!\n", g_img_path);
            printf("v4l2 video capture fail!\n");
            return -1;
        }

        printf("resize - resize \n");
        if(orig_img.cols != ALG_IMAGE_WIDTH || orig_img.rows != ALG_IMAGE_HEIGHT) {
            printf("resize %d %d to %d %d\n", orig_img.cols, orig_img.rows, ALG_IMAGE_WIDTH, ALG_IMAGE_HEIGHT);
            cv::resize(orig_img, img, cv::Size(ALG_IMAGE_WIDTH, ALG_IMAGE_HEIGHT), (0, 0), (0, 0), cv::INTER_LINEAR);
        }
        
        /*********
        * rknn model run
        **********/

        algorithm_node_run(&ctx, img.data , &model_input_size, &model_ninput);

        auto end_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        /*********
        * Logging
        **********/
        log_memory();

        for(int i=0;i<7;i++) {
            read_from_file(filepaths[i]);
        }
        float fps = 0;
        if ((end_milli-start_milli) > 0) {
            fps = 1000/((float) (end_milli-start_milli));
        }
        log_file << fps << ", " << start_milli <<  ", " << end_milli;
        algorithm_node_post(&ctx, RKNN_model, &model_noutput, orig_img);

        printf("post processing \n");

        usleep((80 - (start_milli-end_milli)) *1000);// limit to 10 fps = 100 ms  , running only one camera here.... Value used is 80 to compensate for fps calculation just after!
            
        auto real_end_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if ((real_end_milli-start_milli) > 0) {
            fps = 1000/((float) (real_end_milli-start_milli));
        }
        log_file <<  ", " << real_end_milli << "\n";
        std::cout <<  "fps" << fps << "\n\n";

        //release matricies 
        //orig_img.release();
        //img.release();
        //raw_img.release();

        //avoid segfault (DEBUG only)
        //break;
    }

    stop_capturing();
    uninit_device();
    close_device();

    printf("release rknn\n");
    // Release
    if (ctx >= 0)
    {
        rknn_destroy(ctx);
    }
    printf("release model\n");
    if (RKNN_model)
    {
        free(RKNN_model);
    }
    
    printf("release camera buffer\n");
    //release 
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
