//  Copyright (C) 2023 PRM Engineering Solutions - All Rights Reserved
//  You may use, distribute and modify this code under the
//  terms of the commercial license.
//
//  You should have received a copy of the commercial license with
//  this file. If not, please write to info@prmsolutions.com.au, or
//  visit https://prmengineering.com.au/contact/:
///
///
/// \brief
///
/// \file mqtt_handler.cc
///

#include "awsiot/mqtt_handler.h"

#include <aws/crt/Api.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>

#include <chrono>
#include <iostream>
#include <queue>
#include <string>
#include <thread>

#include "awsiot/component_update_handler.h"
#include "awsiot/lifecycle_handler.h"
#include "glog/logging.h"  // for DLOG, COMPACT_GOOGLE_LOG_INFO, LogMessage, COMPACT_GOOGLE_LOG_F...
#include "visionai/sentinel_datatypes.h"

using namespace Aws::Crt;
using namespace Aws::Greengrass;

namespace sv {
/// Enable the locally connected display
DEFINE_bool(enable_heartbeat, false, "Enable Sentinel Vision to publish heartbeat messages to the cloud");

MqttHandler::MqttHandler() { Setup(); }

MqttHandler::~MqttHandler() {
  LOG(INFO) << "Closing the MQTT handler";

  if (thread_ != nullptr) {
    if (thread_->joinable()) {
      LOG(INFO) << "Closing the worker thread";
      is_running_ = false;
      thread_->join();
    } else {
      LOG(INFO) << "Worker thread is not joinable";
    }
  }
}

sv::Status MqttHandler::Setup() {
  // Retrieve environment variables
  device_name_ = getenv("AWS_IOT_THING_NAME");
  ggc_version_ = getenv("GGC_VERSION");

  // Check if required environment variables are not set
  if (!device_name_ || !ggc_version_) {
    // Log an error and disable datalogging
    LOG(ERROR) << "AWS Environment variables 'AWS_IOT_THING_NAME' 'GGC_VERSION' not set. Datalogging is disabled";
    return sv::Status::kFailure;
  }
  // Log the retrieved thing name and GGC version
  DLOG(INFO) << "Thing name: " << device_name_ << "\nGGC version: " << ggc_version_;

  // Check if MQTT setup is successful
  if (MqttHandler::SetupMQTT() != sv::Status::kSuccess) {
    // Log an error and disable datalogging
    LOG(ERROR) << "Could not initialize IoT Core. MQTT publish is disabled";
    return sv::Status::kFailure;
  }

  // Create a std::unique_ptr to manage the std::thread object and pass the lambda
  thread_ = std::make_unique<std::thread>(&MqttHandler::MainThread, this);

  return sv::Status::kSuccess;
}

sv::Status MqttHandler::SetupMQTT() {
  // Initialize ApiHandle
  apiHandle_ = std::make_unique<Aws::Crt::ApiHandle>(DefaultAllocator());

  // Initialize EventLoopGroup
  event_loop_group_ = std::make_unique<Aws::Crt::Io::EventLoopGroup>(1);

  // Initialize DefaultHostResolver
  socket_resolver_ = std::make_unique<Aws::Crt::Io::DefaultHostResolver>(*event_loop_group_, 64, 30);

  // Initialize ClientBootstrap
  client_bootstrap_ = std::make_unique<Aws::Crt::Io::ClientBootstrap>(*event_loop_group_, *socket_resolver_);

  // Initialize IpcClientLifecycleHandler
  ipc_lifecycle_handler_ = std::make_shared<sv::IpcClientLifecycleHandler>();

  // Initialize GreengrassCoreIpcClient
  ipc_client_ = std::make_shared<Aws::Greengrass::GreengrassCoreIpcClient>(*client_bootstrap_);

  // Connect to IPC
  // Check IPC connection status
  if (auto connectionStatus = ipc_client_->Connect(*ipc_lifecycle_handler_).get(); !connectionStatus) {
    LOG(ERROR) << "Failed to establish IPC connection: " << connectionStatus.StatusToString();
    return sv::Status::kFailure;
  } else {
    // MQTT setup successful, mark as connected
    is_connected_ = true;
    is_running_ = true;
  }

  ipc_lifecycle_handler_ = std::make_unique<sv::IpcClientLifecycleHandler>();

  return sv::Status::kSuccess;
}

sv::Status MqttHandler::SubscribeToComponentUpdates() {
  Aws::Greengrass::SubscribeToComponentUpdatesRequest request;
  ipc_update_handler_ = std::make_shared<sv::IpcClientComponentUpdatesStreamHandler>();
  ipc_update_handler_->AddIPCClient(ipc_client_);
  ipc_update_operation_ = ipc_client_->NewSubscribeToComponentUpdates(ipc_update_handler_);
  auto activate = ipc_update_operation_->Activate(request, nullptr);
  activate.wait();

  auto responseFuture = ipc_update_operation_->GetResult();
  if (responseFuture.wait_for(std::chrono::seconds(timeout_)) == std::future_status::timeout) {
    LOG(ERROR) << "Operation timed out while waiting for response from Greengrass Core." << std::endl;
    return sv::Status::kFailure;
  }

  auto response = responseFuture.get();
  if (response) {
    LOG(INFO) << "Successfully subscribed component updates: " << std::endl;
  } else {
    // An error occurred.
    LOG(ERROR) << "Failed to subscribe component updates: " << std::endl;
    auto errorType = response.GetResultType();
    if (errorType == OPERATION_ERROR) {
      auto *error = response.GetOperationError();
      LOG(ERROR) << "Operation error: " << error->GetMessage().value() << std::endl;
      return sv::Status::kFailure;

    } else {
      LOG(ERROR) << "RPC error: " << response.GetRpcError() << std::endl;
      return sv::Status::kFailure;
    }
  }
  return sv::Status::kSuccess;
}

void MqttHandler::MainThread() {
  // subscribe to component updates then keep the thread alive to process the queue
  SubscribeToComponentUpdates();

  // Keep the main thread alive, or the process will exit.
  int32_t iteration_count = 0;
  int32_t heartbeat_count = 0;

  while (is_running_) {
    // Process all elements in the FIFO queue
    while (!queue_.empty()) {
      std::scoped_lock lock(queueMutex);
      // Dequeue element from the FIFO queue
      auto [dequeuedTopic, dequeuedMessage] = queue_.front();
      queue_.pop();

      // Process dequeued elements by publishing to IoT Core
      MqttHandler::PublishToIoTCore(dequeuedMessage, dequeuedTopic);
    }

    // Publish heartbeat (if enabled) every 10 iterations
    if (FLAGS_enable_heartbeat && (iteration_count % (10 * heartbeat_period_)) == 0) {
      // Get the current timestamp in epoch time
      auto current_time = std::chrono::system_clock::now();
      auto epoch_time = std::chrono::duration_cast<std::chrono::seconds>(current_time.time_since_epoch()).count();

      // Create heartbeat message payload
      std::string message_payload = R"({"message_number": )" + std::to_string(heartbeat_count) +
                                    R"(, "origin_device": ")" + device_name_ + R"(", "timestamp": )" +
                                    std::to_string(epoch_time) + R"(})";
      heartbeat_count++;
      // Publish heartbeat telemetry message~
      MqttHandler::QueueMessage(message_payload, "heartbeat");
    }

    iteration_count++;

    // Sleep for 100 milliseconds before the next iteration
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

sv::Status MqttHandler::QueueMessage(const std::string &message_payload, const std::string &topic_name) {
  // Enqueue
  if (!is_connected_) {
    LOG(ERROR) << "Could not publish message, Datalogging is not initalized";
    return sv::Status::kFailure;
  }

  std::scoped_lock lock(queueMutex);
  queue_.emplace(topic_name, message_payload);

  return sv::Status::kSuccess;
}

sv::Status MqttHandler::PublishToIoTCore(const std::string_view &message_payload, const std::string_view &topic_name) {
  auto topic_path = std::string("data/") + std::string(device_name_) + "/" + std::string(topic_name);

  // Publish to topic
  auto publish_operation = ipc_client_->NewPublishToIoTCore();
  PublishToIoTCoreRequest publish_request;
  publish_request.SetTopicName(topic_path.c_str());
  Vector<uint8_t> payload(message_payload.begin(), message_payload.end());
  publish_request.SetPayload(payload);
  publish_request.SetQos(Aws::Greengrass::QOS::QOS_AT_LEAST_ONCE);

  // Publish
  if (auto request_status = publish_operation->Activate(publish_request).get(); !request_status)
    LOG(ERROR) << "Failed to publish to " << topic_path.c_str() << " topic with error "
               << request_status.StatusToString().c_str();

  auto publish_result_future = publish_operation->GetResult();

  if (auto publish_result = publish_result_future.get(); publish_result) {
    const auto *response = publish_result.GetOperationResponse();
    (void)response;
  } else {
    auto error_type = publish_result.GetResultType();
    if (error_type == OPERATION_ERROR) {
      OperationError *error = publish_result.GetOperationError();
      if (error->GetMessage().has_value()) {
        LOG(ERROR) << "Greengrass Core responded with an error: " << error->GetMessage().value().c_str();
        return sv::Status::kError;
      }
    } else {
      LOG(ERROR) << "Attempting to receive the response from the server failed with error code %s\n"
                 << publish_result.GetRpcError().StatusToString().c_str();
      return sv::Status::kError;
    }
  }

  return sv::Status::kSuccess;
}

}  // namespace sv