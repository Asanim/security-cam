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

#ifndef ALG_INFERENCE_CORAL_H_
#define ALG_INFERENCE_CORAL_H_

#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// Open cv libs
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "tensorflow/lite/interpreter.h"
#include "visionai/sentinel_datatypes.h" // for Status

namespace sv
{
  /// Enum representing common success / failure status.
  enum class Status
  {
    kSuccess = 0,
    kFailure,
    kError,
    kTimeout
  };

  ///
  /// \brief The InferenceChannel class
  ///
  ///
  class InferenceChannel
  {
  public:
    ///
    /// \brief Construct a new Inference Channel object
    ///
    /// \param channel_id The ID of the IPU channel to use for inference
    /// \param camera_identity The ID of the IPU channel to use for inference
    ///
    explicit InferenceChannel(uint32_t channel_id, const CameraIdentity &camera_identity);

    ///
    /// \brief Destroy the Inference Channel object
    ///
    ///
    ~InferenceChannel();

    ///
    /// \brief Image recognition function
    /// This function performs image recognition on a given RGB image
    /// using the InferenceChannel class.
    /// The function takes a pointer to the RGB image data
    /// Results must be collected using GetInferenceResults
    ///
    /// \param image A pointer to the image data
    ///
    /// \return StatusType
    ///
    sc::Status RunImageInference(Image *image);

    ///
    /// \brief Get the Inference Results attribute
    /// This function retrieves the results of the inference operation.
    ///
    /// \param detections A pointer to a vector of Detection objects
    ///
    /// \return StatusType
    ///
    sc::Status GetInferenceResults(std::vector<sc::Detection> *detections) const;

    ///
    /// \brief Get the Label Path attribute
    ///
    /// \return std::string
    ///
    static std::string GetLabelPath() { return kLabelPath; }

    ///
    /// \brief Get the input image size
    ///
    /// \param width
    /// \param height
    /// \return sc::Status
    ///
    sc::Status GetInputImageSize(uint16_t *width, uint16_t *height) const;

  private:
    /// Indicates whether the IPU device has been initialized
    static bool isInitalized;
    /// Interpreter object
    static std::unique_ptr<tflite::Interpreter> interpreter;
    /// The IPU loadable firmware path
    static std::string kFirmwarePath;
    /// The IPU model path
    static std::string kModelImgPath;
    /// The IPU label path
    static std::string kLabelPath;
    /// The label array size
    int32_t label_class_count_ = 100;
    /// The actual class labels
    std::vector<std::string> class_labels_;
  };

} // namespace sv

#endif // ALG_INFERENCE_CORAL_H_
