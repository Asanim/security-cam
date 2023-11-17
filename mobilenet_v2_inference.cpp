#include "yolo_inference.h"

MobilenetV2SSDInference::MobilenetV2SSDInference(const std::string& enginePath) {
    runtime = nvinfer1::createInferRuntime(gLogger);
    std::ifstream engineFile(enginePath, std::ios::binary);
    if (!engineFile) {
        throw std::runtime_error("Error: Could not open engine file.");
    }

    engineFile.seekg(0, engineFile.end);
    size_t engineSize = engineFile.tellg();
    engineFile.seekg(0, engineFile.beg);
    std::vector<char> engineData(engineSize);
    engineFile.read(engineData.data(), engineSize);
    engineFile.close();

    engine = runtime->deserializeCudaEngine(engineData.data(), engineSize, nullptr);
    if (!engine) {
        throw std::runtime_error("Error: Could not deserialize engine.");
    }

    context = engine->createExecutionContext();
    if (!context) {
        throw std::runtime_error("Error: Could not create execution context.");
    }

    inputDim = engine->getBindingDimensions(0);
    outputDim = engine->getBindingDimensions(1);
    inputSize = std::accumulate(inputDim.d, inputDim.d + inputDim.nbDims, 1, std::multiplies<int>());
    outputSize = std::accumulate(outputDim.d, outputDim.d + outputDim.nbDims, 1, std::multiplies<int>());

    cudaMalloc(&buffers[0], inputSize * sizeof(float));
    cudaMalloc(&buffers[1], outputSize * sizeof(float));
}

MobilenetV2SSDInference::~MobilenetV2SSDInference() {
    cudaFree(buffers[0]);
    cudaFree(buffers[1]);
    context->destroy();
    engine->destroy();
    runtime->destroy();
}

std::vector<cv::Mat> MobilenetV2SSDInference::preProcess(const cv::Mat& frame) {
    cv::Mat resizedFrame, rgbFrame, floatFrame;
    cv::resize(frame, resizedFrame, cv::Size(inputDim.d[2], inputDim.d[1]));
    cv::cvtColor(resizedFrame, rgbFrame, cv::COLOR_BGR2RGB);
    rgbFrame.convertTo(floatFrame, CV_32FC3, 1.0 / 255);
    return {floatFrame};
}

int MobilenetV2SSDInference::infer() {
    // Perform inference using TensorRT
    context->executeV2(buffers);
    return 0;
}

std::vector<std::vector<std::vector<float>>> MobilenetV2SSDInference::postProcess(float iouThresh, float confThresh) {
    // Implement post-processing logic here
    std::vector<std::vector<std::vector<float>>> detections;
    // ... (post-processing code)
    return detections;
}

void MobilenetV2SSDInference::drawDetections(cv::Mat& frame, const std::vector<std::vector<float>>& detections) {
    for (const auto& detection : detections) {
        float confidence = detection[2];
        if (confidence > 0.5) { // Confidence threshold
            int x1 = static_cast<int>(detection[3] * frame.cols);
            int y1 = static_cast<int>(detection[4] * frame.rows);
            int x2 = static_cast<int>(detection[5] * frame.cols);
            int y2 = static_cast<int>(detection[6] * frame.rows);
            cv::rectangle(frame, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0), 2);
        }
    }
}
