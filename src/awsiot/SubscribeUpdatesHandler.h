#ifndef SUBSCRIBE_UPDATES_HANDLER_H
#define SUBSCRIBE_UPDATES_HANDLER_H

#include <iostream>
#include <aws/greengrass/GreengrassCoreIpcModel.h>

class SubscribeUpdatesHandler : public Aws::Greengrass::ipc::SubscribeToComponentUpdatesStreamHandler {
public:
    virtual ~SubscribeUpdatesHandler();

    bool IsUpdateReady();

private:
    GreengrassCoreIpcClient *ipcClient;

    void OnStreamEvent(Aws::Greengrass::ComponentUpdatePolicyEvents *response) override;
    void deferUpdate(Aws::Crt::String deploymentId);
    void acknowledgeUpdate(Aws::Crt::String deploymentId);
    bool OnStreamError(Aws::Greengrass::OperationError *error) override;
    bool OnStreamError(Aws::Greengrass::ServiceError *error) override;
    bool OnStreamError(Aws::Greengrass::ResourceNotFoundError *error) override;
    void OnStreamClosed() override;
};

#endif // SUBSCRIBE_UPDATES_HANDLER_H
