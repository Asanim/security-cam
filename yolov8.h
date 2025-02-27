/*
 * SPDX-FileCopyrightText: Copyright (c) 2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <cuda_runtime.h>
#include "NvInfer.h"
#include <vector>
#include <NvInferPlugin.h>
#include <fstream>
#include <algorithm>
#include <numeric>
// opencv for preprocessing & postprocessing
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class Yolov8 {
public:
    Yolov8(std::string engine_path);

    std::vector<cv::Mat> preProcess(std::vector<cv::Mat> &cv_img);

    int infer();

    std::vector<std::vector<std::vector<float>>> PostProcess(float iou_thres = 0.45f, float conf_thres = 0.25f);

    nvinfer1::Dims getInputDim();

    nvinfer1::Dims getOutputDim();

    static int DrawBoxesonGraph(cv::Mat &bgr_img, std::vector<std::vector<float>> nmsresult);

private:
    int pushImg(void *imgBuffer, int numImg, bool fromCPU = true);

    std::vector<std::vector<std::vector<float>>> decode_yolov8_result(float conf_thres);

    std::vector<std::vector<std::vector<float>>> yolov8_nms(std::vector<std::vector<std::vector<float>>> &bboxes, float iou_thres);

    std::vector<std::vector<float>> nms(std::vector<std::vector<float>> &bboxes, float iou_thres);

    void ReportArgs();

private:
    int mImgPushed;
    int mMaxBatchSize;
    bool mDynamicBatch;

    std::unique_ptr<CUstream_st, StreamDeleter> mStream;
    std::unique_ptr<CUevent_st, EventDeleter> mEvent;

    std::unique_ptr<nvinfer1::IRuntime, TrtDeleter<nvinfer1::IRuntime>> mRuntime;
    std::unique_ptr<nvinfer1::ICudaEngine, TrtDeleter<nvinfer1::ICudaEngine>> mEngine;
    std::unique_ptr<nvinfer1::IExecutionContext, TrtDeleter<nvinfer1::IExecutionContext>> mContext;
    std::vector<std::unique_ptr<char, CuMemDeleter<char>>> mBindings;

    std::vector<void *> mBindingArray;
    std::vector<float> mHostOutputBuffer;
    std::vector<float> mHostNMSBuffer;

    std::string mEnginePath;
    nvinfer1::Dims mInputDim;
    nvinfer1::Dims mOutputDim;
    int mImgBufferSize;

    cudaGraph_t mGraph{};
    cudaGraphExec_t mGraphExec{};

    std::vector<std::vector<float>> md2i;

    bool mCudaGraphEnabled;

    unsigned long long mLast_inference_time;
    unsigned long long mTotal_inference_time;
    int mInference_count;

public:
    int imgProcessed() { return mInference_count; };
};