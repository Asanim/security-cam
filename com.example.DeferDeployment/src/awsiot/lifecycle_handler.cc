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
/// \file lifecycle_handler.cc
///

#include "lifecycle_handler.h"
#include "glog/logging.h"  // for DLOG, COMPACT_GOOGLE_LOG_INFO, LogMessage, COMPACT_GOOGLE_LOG_F...

namespace sv {

void IpcClientLifecycleHandler::OnConnectCallback() {
  DLOG(INFO) << "Connected to IPC service." << std::endl;
  // Handle connection to IPC service.
}

void IpcClientLifecycleHandler::OnDisconnectCallback(RpcError error) {
  LOG(WARNING) << "Disconnected from IPC service. Error: " << error.StatusToString() << std::endl;
  // Handle disconnection from IPC service.
}

bool IpcClientLifecycleHandler::OnErrorCallback(RpcError error) {
  LOG(ERROR) << "IPC service connection error: " << error.StatusToString() << std::endl;

  // Handle IPC service connection error.
  return true;
}
}  // namespace sv
