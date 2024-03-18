///
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
/// \file component_update_handler.h
///

#ifndef AWSIOT_COMPONENT_UPDATE_HANDLER_H_
#define AWSIOT_COMPONENT_UPDATE_HANDLER_H_

#include <aws/crt/Api.h>
#include <aws/crt/io/HostResolver.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>

#include <iostream>

#include "awsiot/mqtt_handler.h"
#include "glog/logging.h"  // for DLOG, COMPACT_GOOGLE_LOG_INFO, LogMessage, COMPACT_GOOGLE_LOG_F...
#include "hardware/ignition_subscriber.h"
#include "visionai/sentinel_datatypes.h"

namespace sv {

/// \brief Implements the lifecycle callbacks for an IPC client connection.
///
/// This class is derived from IpcClientComponentUpdatesStreamHandler and provides
/// implementations for the OnConnectCallback, OnDisconnectCallback, and
/// OnErrorCallback methods.
class IpcClientComponentUpdatesStreamHandler : public Aws::Greengrass::SubscribeToComponentUpdatesStreamHandler,
                                               public IgnitionSubscriber {
 public:
  /// \brief Destructor
  virtual ~IpcClientComponentUpdatesStreamHandler() = default;

  ///
  /// \brief Run when the ignition_state changes. Used to save SystemStatus on shutdown
  ///
  /// \param ignition_state The ignition state of the system
  /// \param low_voltage True if the system is in low voltage mode
  ///
  void UpdateIgnitionState(const IgnitionState ignition_state, const bool low_voltage) override;

  ///
  /// \brief Adds a pointer to an instance of MqttHandler for managing MQTT communication.
  /// \param mqtt_handler Pointer to an instance of MqttHandler for managing MQTT communication.
  ///
  void AddIPCClient(std::shared_ptr<Aws::Greengrass::GreengrassCoreIpcClient> ipc_client);

  void OnStreamEvent(Aws::Greengrass::ComponentUpdatePolicyEvents *response) override;

  bool OnStreamError(Aws::Eventstreamrpc::RpcError rpcError) override;

  bool OnStreamError(Aws::Greengrass::ServiceError *operationError) override;

  bool OnStreamError(Aws::Greengrass::ResourceNotFoundError *operationError) override;

  bool OnStreamError(Aws::Eventstreamrpc::OperationError *operationError) override;

  sv::Status DeferSystemComponentUpdate(std::string deployment_id, int32_t recheck_after_ms);

 private:
  std::shared_ptr<Aws::Greengrass::GreengrassCoreIpcClient>
      ipc_client_;  ///< Pointer to an instance of GreengrassCoreIpcClient for Greengrass Core IPC communication.

  bool is_update_safe_ = false;

  int32_t component_update_recheck_ms_ = 1000;

  std::string deployment_id_ = "";
};
}  // namespace sv

#endif  // AWSIOT_COMPONENT_UPDATE_HANDLER_H_
