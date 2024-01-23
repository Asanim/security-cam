#ifndef IPC_CLIENT_LIFECYCLE_HANDLER_H
#define IPC_CLIENT_LIFECYCLE_HANDLER_H

#include <iostream>
#include <aws/greengrass/ipc/ConnectionLifecycleHandler.h>

class IpcClientLifecycleHandler : public aws::greengrass::ipc::ConnectionLifecycleHandler {
public:
    void OnConnectCallback() override;
    void OnDisconnectCallback(RpcError error) override;
    bool OnErrorCallback(RpcError error) override;
};

#endif // IPC_CLIENT_LIFECYCLE_HANDLER_H
