#ifndef SUBSCRIBE_UPDATES_HANDLER_H
#define SUBSCRIBE_UPDATES_HANDLER_H

#include <iostream>
#include <aws/greengrass/ipc/SubscribeToComponentUpdatesStreamHandler.h>

class SubscribeUpdatesHandler : public aws::greengrass::ipc::SubscribeToComponentUpdatesStreamHandler {
public:
    virtual ~SubscribeUpdatesHandler();

    bool IsUpdateReady();

private:
    GreengrassCoreIpcClient *ipcClient;

    void OnStreamEvent(ComponentUpdatePolicyEvents *response) override;
    void deferUpdate(Aws::Crt::String deploymentId);
    void acknowledgeUpdate(Aws::Crt::String deploymentId);
    bool OnStreamError(OperationError *error) override;
    bool OnStreamError(ServiceError *error) override;
    bool OnStreamError(ResourceNotFoundError *error) override;
    void OnStreamClosed() override;
};

#endif // SUBSCRIBE_UPDATES_HANDLER_H
