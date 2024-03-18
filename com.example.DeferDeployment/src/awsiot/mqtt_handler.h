//  Copyright (C) 2023 PRM Engineering Solutions - All Rights Reserved
//  You may use, distribute and modify this code under the
//  terms of the commercial license.
//
//  You should have received a copy of the commercial license with
//  this file. If not, please write to info@prmsolutions.com.au, or
//  visit https://prmengineering.com.au/contact/:
///
///
/// \brief Manages communication with AWS IoT Greengrass Nucleus
///
/// \file mqtt_handler.h
///

#ifndef AWSIOT_MQTT_HANDLER_H_
#define AWSIOT_MQTT_HANDLER_H_

#include <aws/crt/Api.h>
#include <aws/crt/io/HostResolver.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <utility>

#include "awsiot/component_update_handler.h"
#include "awsiot/lifecycle_handler.h"
#include "glog/logging.h"  // for DLOG, COMPACT_GOOGLE_LOG_INFO, LogMessage, COMPACT_GOOGLE_LOG_F...
#include "visionai/sentinel_datatypes.h"

namespace sv {

/// \brief MqttHandler class for handling MQTT communication.
class MqttHandler {
 public:
  /// \brief Constructor for the MqttHandler class.
  MqttHandler();

  /// \brief Destructor for the MqttHandler class.
  ~MqttHandler();

  /// \brief Start the initialization step.
  ///
  /// \return Status indicating the success or failure of the operation.
  sv::Status Setup();

  /// \brief Publish telemetry message to the internal buffer.
  ///
  /// \param message_payload The payload of the message.
  /// \param topic_name The MQTT topic name to which the message is published.
  /// \return Status indicating the success or failure of the operation.
  sv::Status QueueMessage(const std::string& message_payload, const std::string& topic_name);

 private:
  /// \brief Main thread function for handling MQTT operations.
  void MainThread();

  /// \brief Publish telemetry message to the specified MQTT topic.
  ///
  /// \param message_payload The payload of the message.
  /// \param topic_name The MQTT topic name to which the message is published.
  /// \return Status indicating the success or failure of the operation.
  sv::Status PublishToIoTCore(const std::string_view& message_payload, const std::string_view& topic_name);

  /// \brief Set up the MQTT connection and configuration.
  sv::Status SetupMQTT();

  /// \brief Subscribe to updates.
  sv::Status SubscribeToComponentUpdates();

  std::mutex queueMutex;  ///< Queue mutex for queueing messages.

  // Member variables
  std::unique_ptr<std::thread> thread_;   ///< Thread for the main MQTT handling logic.
  std::thread startup_thread_;            ///< Thread for connection to AWS Greengrass and IoT Core.
  std::atomic<bool> is_running_ = false;  ///< Flag indicating whether the MQTT handler is running.
  bool is_connected_ = false;             ///< Flag indicating whether the class has connected to aws iot
  const char* ggc_version_;               ///< Greengrass Core version information.
  std::string message_topic_;             ///< Default MQTT topic for telemetry messages.
  const char* device_name_;               ///< Name of the thing associated with MQTT.
  uint32_t heartbeat_period_ = 900;       ///< heartbeat message period in seconds, (currently send one every 15 mins)

  int32_t timeout_ = 10;  // timeout in seconds

  std::unique_ptr<Aws::Crt::ApiHandle> apiHandle_;  ///< Pointer to an instance of ApiHandle for API interactions.
  std::unique_ptr<Aws::Crt::Io::EventLoopGroup>
      event_loop_group_;  ///< Pointer to an instance of EventLoopGroup for managing event loops.
  std::unique_ptr<Aws::Crt::Io::DefaultHostResolver>
      socket_resolver_;  ///< Pointer to an instance of DefaultHostResolver for socket resolution.
  std::unique_ptr<Aws::Crt::Io::ClientBootstrap>
      client_bootstrap_;  ///< Pointer to an instance of ClientBootstrap for setting up client connections.
  std::shared_ptr<sv::IpcClientLifecycleHandler>
      ipc_lifecycle_handler_;  ///< Pointer to an instance of IpcClientLifecycleHandler for managing IPC client
                               ///< lifecycle.
  std::shared_ptr<Aws::Greengrass::GreengrassCoreIpcClient>
      ipc_client_;  ///< Pointer to an instance of GreengrassCoreIpcClient for Greengrass Core IPC communication.

  std::queue<std::pair<std::string, std::string>>
      queue_;  ///< Instance of FifoQueue for managing a First In First Out (FIFO) queue.
  /// Component update stream handler
  std::shared_ptr<sv::IpcClientComponentUpdatesStreamHandler> ipc_update_handler_;
  std::shared_ptr<Aws::Greengrass::SubscribeToComponentUpdatesOperation> ipc_update_operation_;
};  // class MqttHandler

}  // namespace sv

#endif  // AWSIOT_MQTT_HANDLER_H_
