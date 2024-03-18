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
/// \file lifecycle_handler.h
///

#ifndef AWSIOT_LIFECYCLE_HANDLER_H_
#define AWSIOT_LIFECYCLE_HANDLER_H_

#include <aws/crt/Api.h>
#include <aws/crt/io/HostResolver.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>

#include <iostream>

namespace sv {

/// \brief Implements the lifecycle callbacks for an IPC client connection.
///
/// This class is derived from ConnectionLifecycleHandler and provides
/// implementations for the OnConnectCallback, OnDisconnectCallback, and
/// OnErrorCallback methods.
class IpcClientLifecycleHandler : public ConnectionLifecycleHandler {
 public:
  /// \brief Destructor
  virtual ~IpcClientLifecycleHandler() = default;

  /// \brief Called when the IPC client successfully connects to the service.
  void OnConnectCallback() override;

  /// \brief Called when the IPC client disconnects from the service.
  ///
  /// \param error An object representing the error, if any, during disconnection.
  void OnDisconnectCallback(RpcError error) override;

  /// \brief Called when an error occurs during the IPC client connection.
  ///
  /// \param error An object representing the error that occurred.
  /// \return True if the error is handled, false otherwise.
  bool OnErrorCallback(RpcError error) override;
};
}  // namespace sv

#endif  // AWSIOT_LIFECYCLE_HANDLER_H_
