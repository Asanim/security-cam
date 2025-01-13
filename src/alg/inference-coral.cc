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
 *  \file inference-coral.h
 */


#include "alg/inference_channel.h"

#include <glog/logging.h>  // for DLOG, LogMessage, COMPACT_GOOGLE_LOG_INFO, COMPACT_GOOGLE_LOG_ERROR

#include <fstream>
#include <iostream>
#include <string_view>

#include "edgetpu_c.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"

namespace sc {

// initialize string constants
// A segmentation or hardware fault occurs if these files are not present
std::string InferenceChannel::kFirmwarePath = "/opt/sv/model/ipu_firmware.bin";
std::string InferenceChannel::kModelImgPath = "/opt/sv/model/ssd_mobilenet_v1_coco_2018_01_28_fixed.sim_sgsimg.img";
std::string InferenceChannel::kLabelPath = "/opt/sv/model/ssd_mobilenet_v1_label.txt";
//  const std::string kCoralModelImgPath = "/opt/sv/model/ssd_mobilenet_v2_coco_quant_postprocess_edgetpu.tflite";
// const std::string kCoralModelImgPath = "/opt/sv/model/yolo11n_full_integer_quant_320x224_edgetpu.tflite";
const std::string kCoralModelImgPath = "/opt/sv/model/yolo11n_full_integer_quant_edgetpu.tflite";

bool InferenceChannel::isInitalized = false;
std::unique_ptr<tflite::Interpreter> InferenceChannel::interpreter;

constexpr size_t kBmpFileHeaderSize = 14;
constexpr size_t kBmpInfoHeaderSize = 40;
constexpr size_t kBmpHeaderSize = kBmpFileHeaderSize + kBmpInfoHeaderSize;

int32_t ToInt32(const char p[4]) { return (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0]; }

std::vector<uint8_t> ReadBmpImage(const char *filename, int *out_width = nullptr, int *out_height = nullptr,
                                  int *out_channels = nullptr) {
  assert(filename);

  std::ifstream file(filename, std::ios::binary);
  if (!file) return {};  // Open failed.

  char header[kBmpHeaderSize];
  if (!file.read(header, sizeof(header))) return {};  // Read failed.

  const char *file_header = header;
  const char *info_header = header + kBmpFileHeaderSize;

  if (file_header[0] != 'B' || file_header[1] != 'M') return {};  // Invalid file type.

  const int channels = info_header[14] / 8;
  if (channels != 1 && channels != 3) return {};  // Unsupported bits per pixel.

  if (ToInt32(&info_header[16]) != 0) return {};  // Unsupported compression.

  const uint32_t offset = ToInt32(&file_header[10]);
  if (offset > kBmpHeaderSize && !file.seekg(offset - kBmpHeaderSize, std::ios::cur)) return {};  // Seek failed.

  int width = ToInt32(&info_header[4]);
  if (width < 0) return {};  // Invalid width.

  int height = ToInt32(&info_header[8]);
  const bool top_down = height < 0;
  if (top_down) height = -height;

  const int line_bytes = width * channels;
  const int line_padding_bytes = 4 * ((8 * channels * width + 31) / 32) - line_bytes;
  std::vector<uint8_t> image(line_bytes * height);
  for (int i = 0; i < height; ++i) {
    uint8_t *line = &image[(top_down ? i : (height - 1 - i)) * line_bytes];
    if (!file.read(reinterpret_cast<char *>(line), line_bytes)) return {};  // Read failed.
    if (!file.seekg(line_padding_bytes, std::ios::cur)) return {};          // Seek failed.
    if (channels == 3) {
      for (int j = 0; j < width; ++j) std::swap(line[3 * j], line[3 * j + 2]);
    }
  }

  if (out_width) *out_width = width;
  if (out_height) *out_height = height;
  if (out_channels) *out_channels = channels;
  return image;
}

InferenceChannel::InferenceChannel(uint32_t channel_id, const CameraIdentity &camera_identity)
    : ipu_channel_id_(channel_id), camera_identity_(camera_identity) {
  if (Init() != sc::Status::kSuccess) {
    LOG(ERROR) << "Init failed!";
  }
  if (isInitalized) {
    LOG(INFO) << "InferenceChannel is already initialized";
  } else {
    // Find TPU device.
    // size_t num_devices;
    // std::unique_ptr<edgetpu_device, decltype(&edgetpu_free_devices)> devices(edgetpu_list_devices(&num_devices),
    //                                                                          &edgetpu_free_devices);
    // LOG(INFO) << "VER: 0.0.5" << std::endl;

    // if (num_devices == 0) {
    //   LOG(ERROR) << "No connected TPU found" << std::endl;
    //   return;
    // }
    // const auto &device = devices.get()[0];

    // LOG(INFO) << "Found TPU at " << device.path << std::endl;

    // // Load model.
    // auto model = tflite::FlatBufferModel::BuildFromFile(kCoralModelImgPath.c_str());
    // if (!model) {
    //   LOG(ERROR) << "Cannot read model from " << kCoralModelImgPath << std::endl;
    //   return;
    // }

    // LOG(INFO) << "Model: " << kCoralModelImgPath.c_str() << " buffers" << std::endl;

    // // Create interpreter.
    // tflite::ops::builtin::BuiltinOpResolver resolver;
    // if (tflite::InterpreterBuilder(*model, resolver)(&interpreter) != kTfLiteOk) {
    //   LOG(ERROR) << "Cannot create interpreter" << std::endl;
    //   return;
    // }

    // LOG(INFO) << "Interpreter: " << interpreter->tensors_size() << " tensors" << std::endl;

    // auto *delegate = edgetpu_create_delegate(device.type, device.path, nullptr, 0);

    // LOG(INFO) << "Delegate: " << device.type << " TPU at " << device.path << std::endl;
    // interpreter->ModifyGraphWithDelegate(delegate);

    // LOG(INFO) << "ModifyGraphWithDelegate" << std::endl;

    // // Allocate tensors.
    // if (interpreter->AllocateTensors() != kTfLiteOk) {
    //   LOG(ERROR) << "Cannot allocate interpreter tensors" << std::endl;
    //   return;
    // }

    // LOG(INFO) << "Tensors allocated: " << interpreter->tensors_size() << " allocated" << std::endl;

    // // Set interpreter input.
    // const auto *input_tensor = interpreter->input_tensor(0);
    // LOG(INFO) << "input_tensor->type: " << input_tensor->type << " kTfLiteUInt8: " << kTfLiteUInt8 << std::endl;
    // LOG(INFO) << "image_height: " << kImageHeight << std::endl;
    // LOG(INFO) << "image_width: " << kImageWidth << std::endl;
    // LOG(INFO) << "image_bpp: " << kImageChannels << std::endl;
    // LOG(INFO) << "input_tensor->dims->data[0] (1): " << input_tensor->dims->data[0] << std::endl;
    // LOG(INFO) << "input_tensor->dims->data[1] height: " << input_tensor->dims->data[1] << std::endl;
    // LOG(INFO) << "input_tensor->dims->data[2] widht: " << input_tensor->dims->data[2] << std::endl;
    // LOG(INFO) << "input_tensor->dims->data[3] bpp: " << input_tensor->dims->data[3] << std::endl;

    // LOG(INFO) << "Input tensor: " << input_tensor->dims->data[1] << "x" << input_tensor->dims->data[2] << "x"
    //           << input_tensor->dims->data[3] << std::endl;
    // LOG(INFO) << "InferenceChannel initialization complete! Setting initalized boolean to true";

    // LOG(INFO) << "Address of interpreter: " << &interpreter << std::endl;
    // LOG(INFO) << "Address of the interpreter object: " << interpreter.get() << std::endl;

    InferenceChannel::isInitalized = true;
    LOG(INFO) << "InferenceChannel initialization complete!";
  }
}

InferenceChannel::~InferenceChannel() {
}

sc::Status InferenceChannel::LoadClassesFromFile(const std::string &label_path) {
  std::ifstream label_file_stream(label_path);

  if (!label_file_stream.good()) {
    std::cerr << "File (" << label_path << ") does not exist!\n";
    label_class_count_ = 0;
    return sc::Status::kConfigurationInvalid;
  }

  int32_t n = 0;

  std::string line;
  while (std::getline(label_file_stream, line)) {
    class_labels_.push_back(line);

    if (n >= label_class_count_) {
      label_file_stream.close();
      return sc::Status::kConfigurationInvalid;
    }
    n++;
  }

  label_file_stream.close();
  return sc::Status::kSuccess;
}


sc::Status InferenceChannel::RunImageInference(Image *image) {
  static bool isInit = false;
  static std::unique_ptr<tflite::Interpreter> interpreter;
  static std::unique_ptr<tflite::FlatBufferModel> model;
  static edgetpu_device device;
  static const std::string image_file = "/opt/640x640_column_121923_AP6303280348.bmp";
  // static const std::string image_file = "/opt/test_data/checker300X300.bmp";
  // static const std::string image_file = "/opt/320x224_column.bmp";
  std::vector<uint8_t> image_buffer;

  if (!isInit) {
    LOG(INFO) << "Initializing inference pipeline..." << std::endl;

    // Load model.
    model = tflite::FlatBufferModel::BuildFromFile(kCoralModelImgPath.c_str());
    if (!model) {
      std::cerr << "Failed to load model from: " << kCoralModelImgPath << std::endl;
      return sc::Status::kError;
    }

    // Find Edge TPU device.
    size_t num_devices;
    std::unique_ptr<edgetpu_device, decltype(&edgetpu_free_devices)> devices(edgetpu_list_devices(&num_devices),
                                                                             &edgetpu_free_devices);

    if (num_devices == 0) {
      std::cerr << "No Edge TPU devices found." << std::endl;
      return sc::Status::kError;
    }

    device = devices.get()[0];
    std::cout << "Found Edge TPU device at " << device.path << std::endl;

    // Create the interpreter.
    tflite::ops::builtin::BuiltinOpResolver resolver;
    if (tflite::InterpreterBuilder(*model, resolver)(&interpreter) != kTfLiteOk || !interpreter) {
      std::cerr << "Failed to create interpreter." << std::endl;
      return sc::Status::kError;
    }

    // Attach Edge TPU delegate.
    auto *delegate = edgetpu_create_delegate(device.type, device.path, nullptr, 0);
    if (!delegate) {
      std::cerr << "Failed to create Edge TPU delegate." << std::endl;
      return sc::Status::kError;
    }
    if (interpreter->ModifyGraphWithDelegate(delegate) != kTfLiteOk) {
      std::cerr << "Failed to modify graph with delegate." << std::endl;
      return sc::Status::kError;
    }

    // Allocate tensors.
    if (interpreter->AllocateTensors() != kTfLiteOk) {
      std::cerr << "Failed to allocate tensors." << std::endl;
      return sc::Status::kError;
    }

    // Load image.
    int image_bpp, image_width, image_height;
    image_buffer = ReadBmpImage(image_file.c_str(), &image_width, &image_height, &image_bpp);
    if (image_buffer.empty()) {
      std::cerr << "Cannot read image from " << image_file << std::endl;
    }
    std::cout << "Image: " << image_width << "x" << image_height << "x" << image_bpp << std::endl;

    const auto *input_tensor = interpreter->input_tensor(0);
    if (!input_tensor) {
      std::cerr << "Input tensor is null." << std::endl;
      return sc::Status::kError;
    }

    std::cout << "Initialized interpreter with input tensor dimensions: " << input_tensor->dims->data[1] << "x"
               << input_tensor->dims->data[2] << "x" << input_tensor->dims->data[3] << std::endl;

    isInit = true;
  }

  // Ensure image is valid.
  if (!image) {
    std::cerr << "Input image is null." << std::endl;
    return sc::Status::kError;
  }

  std::cout << "Preparing input tensor..." << std::endl;
  auto copy_time = std::chrono::high_resolution_clock::now();

  // Copy image data to input tensor.
  // std::copy(image->data.begin(), image->data.end(), interpreter->typed_input_tensor<uint8_t>(0));
  std::copy(image_buffer.begin(), image_buffer.end(), interpreter->typed_input_tensor<int8_t>(0));
  std::cout << "Input tensor data copied. Invoking inference..." << std::endl;

  // Measure inference time.
  auto start_time = std::chrono::high_resolution_clock::now();

  // Run inference.
  if (interpreter->Invoke() != kTfLiteOk) {
    std::cerr << "Failed to invoke interpreter." << std::endl;
    return sc::Status::kError;
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto inference_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
  auto copy_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(start_time - copy_time).count();

  std::cout << "Image copied in " << copy_time_ms << " ms." << std::endl;
  std::cout << "Inference completed successfully in " << inference_time_ms << " ms." << std::endl;

  std::cout << "Inference completed successfully." << std::endl;
  return sc::Status::kSuccess;
}

sc::Status InferenceChannel::GetInferenceResults(std::vector<sc::Detection> *detections) const {
  // Clear the detections vector
  detections->clear();

  return sc::Status::kSuccess;
}

sc::Status InferenceChannel::GetInputImageSize(uint16_t *width, uint16_t *height) const {
  *height = static_cast<uint16_t>(608);
  // io_descriptor_.astMI_InputTensorDescs[0].u32TensorShape[1]);  // 300 (for coco ssd mobilenet v1)
  *width = static_cast<uint16_t>(608);
  // io_descriptor_.astMI_InputTensorDescs[0].u32TensorShape[2]);  // 300 (for coco ssd mobilenet v1)
  return sc::Status::kSuccess;
}
}  // namespace sc
