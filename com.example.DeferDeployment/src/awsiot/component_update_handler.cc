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
/// \file component_update_handler.cc
///

#include "awsiot/component_update_handler.h"

#include <aws/crt/Api.h>
#include <aws/crt/io/HostResolver.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>

#include "awsiot/mqtt_handler.h"
#include "glog/logging.h"  // for DLOG, COMPACT_GOOGLE_LOG_INFO, LogMessage, COMPACT_GOOGLE_LOG_F...
#include "hardware/ignition_subscriber.h"
#include "settings/database.h"
#include "visionai/sentinel_datatypes.h"

namespace sv {

void IpcClientComponentUpdatesStreamHandler::AddIPCClient(
    std::shared_ptr<Aws::Greengrass::GreengrassCoreIpcClient> ipc_client) {
  ipc_client_ = ipc_client;
}

void IpcClientComponentUpdatesStreamHandler::UpdateIgnitionState(const IgnitionState ignition_state,
                                                                 const bool low_voltage) {
  if (low_voltage) {
    LOG(INFO) << "Low Voltage Shutdown - do not update the device";
    is_update_safe_ = false;
  } else if (ignition_state == IgnitionState::kIgnitionShutDown && !low_voltage) {
    DLOG(INFO) << "Ignition is off, we can apply an update now";
    is_update_safe_ = true;
  }
}

void IpcClientComponentUpdatesStreamHandler::OnStreamEvent(Aws::Greengrass::ComponentUpdatePolicyEvents *response) {
  LOG(INFO) << "Received component update, GetPreUpdateEvent: " << response->GetPreUpdateEvent().has_value()
            << " GetPostUpdateEvent: " << response->GetPostUpdateEvent().has_value() << std::endl;

  if (response->GetPreUpdateEvent().has_value()) {
    LOG(INFO) << "Received component update, event ID: " << response->GetPreUpdateEvent()->GetDeploymentId()->c_str()
              << std::endl;

    // defer the update
    if (is_update_safe_) {
      DeferSystemComponentUpdate(std::string(response->GetPreUpdateEvent()->GetDeploymentId()->c_str()),
                                 component_update_recheck_ms_);
    } else {
      // defer the update for 0ms
      DeferSystemComponentUpdate(std::string(response->GetPreUpdateEvent()->GetDeploymentId()->c_str()), 0);
    }
  }

  if (response->GetPostUpdateEvent().has_value()) {
    LOG(INFO) << "Received component update complete, event ID: "
              << response->GetPostUpdateEvent()->GetDeploymentId()->c_str() << std::endl;
  }
}

bool IpcClientComponentUpdatesStreamHandler::OnStreamError(RpcError rpcError) {
  LOG(ERROR) << "IPC component updates error: " << rpcError.StatusToString() << std::endl;

  // Handle IPC component updates error.
  return true;
}

bool IpcClientComponentUpdatesStreamHandler::OnStreamError(Aws::Greengrass::ServiceError *operationError) {
  LOG(ERROR) << "IPC component updates error: " << operationError->GetMessage()->c_str() << std::endl;

  // Handle IPC component updates error.
  return true;
}

bool IpcClientComponentUpdatesStreamHandler::OnStreamError(Aws::Greengrass::ResourceNotFoundError *operationError) {
  LOG(ERROR) << "IPC component updates error: " << operationError->GetMessage()->c_str() << std::endl;

  // Handle IPC component updates error.
  return true;
}

bool IpcClientComponentUpdatesStreamHandler::OnStreamError(Aws::Eventstreamrpc::OperationError *operationError) {
  LOG(ERROR) << "IPC component updates error: " << operationError->GetMessage()->c_str() << std::endl;

  // Handle IPC component updates error.
  return true;
}

sv::Status IpcClientComponentUpdatesStreamHandler::DeferSystemComponentUpdate(std::string deployment_id,
                                                                              int32_t recheck_after_ms) {
  auto defer_update_operation = ipc_client_->NewDeferComponentUpdate();

  Aws::Greengrass::DeferComponentUpdateRequest defer_update_request;
  defer_update_request.SetDeploymentId("system");
  defer_update_request.SetRecheckAfterMs(recheck_after_ms);

  if (auto request_status = defer_update_operation->Activate(defer_update_request).get(); !request_status)
    LOG(ERROR) << "Failed to defer the deployment " << deployment_id << " with error "
               << request_status.StatusToString().c_str();

  auto defer_update_result_future = defer_update_operation->GetResult();

  if (auto defer_update_result = defer_update_result_future.get(); defer_update_result) {
    const auto *response = defer_update_result.GetOperationResponse();
    (void)response;
  } else {
    auto error_type = defer_update_result.GetResultType();
    if (error_type == OPERATION_ERROR) {
      OperationError *error = defer_update_result.GetOperationError();
      if (error->GetMessage().has_value()) {
        LOG(ERROR) << "Greengrass Core responded with an error: " << error->GetMessage().value().c_str();
        return sv::Status::kError;
      }
    } else {
      LOG(ERROR) << "Attempting to receive the response from the server failed with error code %s\n"
                 << defer_update_result.GetRpcError().StatusToString().c_str();
      return sv::Status::kError;
    }
  }
  return sv::Status::kSuccess;
}
}  // namespace sv
