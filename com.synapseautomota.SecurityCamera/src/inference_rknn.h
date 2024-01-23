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


#ifndef INFERENCE_RKNN_H
#define INFERENCE_RKNN_H

#include <opencv2/opencv.hpp>
#include <rknn_api.h>
#include <iostream>


class InferenceRKNN {

public:
    InferenceRKNN();
    ~InferenceRKNN();

    int init();
    int run(cv::Mat img);
    int post_process();
    int free_rknn();
    
    static unsigned char *load_model(const char *filename, int *model_size);
    static void printRKNNTensor(rknn_tensor_attr *attr);

private:
    rknn_context ctx;
    uint32_t model_input_size;
    unsigned char *model;
    rknn_input_output_num io_num;
    rknn_output outputs[2];
    rknn_input inputs[1];

    const char *ssd_path = "/home/firefly/model/ssd_inception_v2_rk180x.rknn";
};

#endif // INFERENCE_RKNN_H
