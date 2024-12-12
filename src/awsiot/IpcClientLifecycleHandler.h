#ifndef IPC_CLIENT_LIFECYCLE_HANDLER_H
#define IPC_CLIENT_LIFECYCLE_HANDLER_H

#include <aws/crt/Api.h>
#include <aws/crt/io/HostResolver.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>
#include <iostream>

class IpcClientLifecycleHandler : public Aws::Greengrass::ipc::ConnectionLifecycleHandler {
public:
  /// \brief Destructor
  virtual ~IpcClientLifecycleHandler() = default;

    void OnConnectCallback() override;
    void OnDisconnectCallback(RpcError error) override;
    bool OnErrorCallback(RpcError error) override;
};

#endif // IPC_CLIENT_LIFECYCLE_HANDLER_H
