#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <cuda_runtime.h>
#include "NvInfer.h"
#include <vector>
#include <NvInferPlugin.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class MobilenetV2SSDInference {
public:
    MobilenetV2SSDInference(const std::string& enginePath);
    ~MobilenetV2SSDInference();

    std::vector<cv::Mat> preProcess(const cv::Mat& frame);
    int infer();
    std::vector<std::vector<std::vector<float>>> postProcess(float iouThresh = 0.45f, float confThresh = 0.25f);
    static void drawDetections(cv::Mat& frame, const std::vector<std::vector<float>>& detections);

private:
    void* buffers[2];
    nvinfer1::IRuntime* runtime;
    nvinfer1::ICudaEngine* engine;
    nvinfer1::IExecutionContext* context;
    nvinfer1::Dims inputDim;
    nvinfer1::Dims outputDim;
    int inputSize;
    int outputSize;
};
