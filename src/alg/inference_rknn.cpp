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

#include "inference_rknn.h"
#include "model_loader.h" 

#define ALG_IMAGE_HEIGHT 480 // Adjust as needed
#define ALG_IMAGE_WIDTH 640  // Adjust as needed

InferenceRKNN::InferenceRKNN() : ctx(-1), model_input_size(0), model(nullptr) {}

InferenceRKNN::~InferenceRKNN() {
    free_rknn();
}

static unsigned char *InferenceRKNN::load_model(const char *filename, int *model_size) {
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


static void InferenceRKNN::printRKNNTensor(rknn_tensor_attr *attr) {
  printf(
      "index=%d name=%s n_dims=%d dims=[%d %d %d %d] n_elems=%d size=%d fmt=%d "
      "type=%d qnt_type=%d fl=%d zp=%d scale=%f\n",
      attr->index, attr->name, attr->n_dims, attr->dims[3], attr->dims[2],
      attr->dims[1], attr->dims[0], attr->n_elems, attr->size, 0, attr->type,
      attr->qnt_type, attr->fl, attr->zp, attr->scale);
}


int InferenceRKNN::init() {
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

    return 0;
}

int InferenceRKNN::run(cv::Mat img) {
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

int InferenceRKNN::post_process() {
    detect_result_group_t detect_result_group;
    postProcessSSD((float *)(outputs[0].buf), (float *)(outputs[1].buf),
                   ALG_IMAGE_HEIGHT, ALG_IMAGE_WIDTH, &detect_result_group);

    // Release rknn_outputs
    rknn_outputs_release(ctx, 2, outputs);

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

int InferenceRKNN::free_rknn() {
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