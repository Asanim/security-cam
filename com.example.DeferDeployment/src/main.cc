#include <aws/crt/Api.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>

#include <chrono>
#include <iostream>
#include <thread>

using namespace Aws::Crt;
using namespace Aws::Greengrass;

class ComponentUpdateHandler : public SubscribeToComponentUpdatesStreamHandler
{
public:
  virtual ~ComponentUpdateHandler() {}

  std::shared_ptr<Aws::Greengrass::GreengrassCoreIpcClient>
      ipc_client_; ///< Pointer to an instance of GreengrassCoreIpcClient for Greengrass Core IPC communication.

private:
  bool is_update_safe_ = false;
  int32_t component_update_recheck_ms_ = 1000;
  std::string deployment_id_ = "";

  void OnStreamEvent(Aws::Greengrass::ComponentUpdatePolicyEvents *response)
  {
    try
    {
      std::cout << "Received component update, GetPreUpdateEvent: " << response->GetPreUpdateEvent().has_value()
                << " GetPostUpdateEvent: " << response->GetPostUpdateEvent().has_value() << std::endl;

      if (response->GetPreUpdateEvent().has_value())
      {
        std::cout << "Received component update, event ID: " << response->GetPreUpdateEvent()->GetDeploymentId()->c_str()
                  << std::endl;

        // defer the update
        if (is_update_safe_)
        {
          // defer the update for 0ms
          std::cout << "Update is safe, delaying update for 0ms...";
          if (auto deployment_id = response->GetPreUpdateEvent()->GetDeploymentId(); deployment_id.has_value())
            DeferSystemComponentUpdate(*deployment_id, 0);
        }
        else
        {
          std::cout << "Update is not safe, delaying update...";
          if (auto deployment_id = response->GetPreUpdateEvent()->GetDeploymentId(); deployment_id.has_value())
            DeferSystemComponentUpdate(*deployment_id, component_update_recheck_ms_);
        }
      }

      if (response->GetPostUpdateEvent().has_value())
      {
        std::cout << "Received component update complete, event ID: "
                  << response->GetPostUpdateEvent()->GetDeploymentId()->c_str() << std::endl;
      }
    }
    catch (const std::exception &e)
    {
      std::cout << "Exception caught: " << e.what() << std::endl;
    }
  }

  bool OnStreamError(RpcError rpcError)
  {
    std::cout << "IPC component updates error: " << rpcError.StatusToString() << std::endl;

    // Handle IPC component updates error.
    return true;
  }

  bool OnStreamError(Aws::Greengrass::ServiceError *operationError)
  {
    std::cout << "IPC component updates error: " << operationError->GetMessage()->c_str() << std::endl;

    // Handle IPC component updates error.
    return true;
  }

  bool OnStreamError(Aws::Greengrass::ResourceNotFoundError *operationError)
  {
    std::cout << "IPC component updates error: " << operationError->GetMessage()->c_str() << std::endl;

    // Handle IPC component updates error.
    return true;
  }

  bool OnStreamError(Aws::Eventstreamrpc::OperationError *operationError)
  {
    std::cout << "IPC component updates error: " << operationError->GetMessage()->c_str() << std::endl;

    // Handle IPC component updates error.
    return true;
  }

  void DeferSystemComponentUpdate(Aws::Crt::String &deployment_id,
                                  int32_t recheck_after_ms)
  {
    std::cout << "Building defer deployment request" << std::endl;
    auto defer_update_request = Aws::Greengrass::DeferComponentUpdateRequest();
    defer_update_request.SetDeploymentId(deployment_id);
    defer_update_request.SetRecheckAfterMs(1800000);
    defer_update_request.SetMessage(Aws::Crt::String("Sentinel Vision: Update is not safe, deferring until safe."));

    std::cout << "Create defer_update_operation" << std::endl;
    auto defer_update_operation = ipc_client_->NewDeferComponentUpdate();
    std::cout << "Activate defer_update_operation" << std::endl;
    auto request_status = defer_update_operation->Activate(defer_update_request, nullptr);
    std::cout << "Wait for result" << std::endl;
    request_status.wait();

    std::cout << "Get Result" << std::endl;
    auto publish_result_future = defer_update_operation->GetResult();

    if (publish_result_future.wait_for(std::chrono::seconds(10)) == std::future_status::timeout) {
        std::cerr << "Operation timed out while waiting for response from Greengrass Core." << std::endl;
        exit(-1);
    }

    std::cout << "Check Result" << std::endl;
    if (auto publish_result = publish_result_future.get(); publish_result)
    {
      const auto *response = publish_result.GetOperationResponse();
      (void)response;W
      std::cout << "Defer Success" << std::endl;
    }
    else
    {
      auto error_type = publish_result.GetResultType();
      if (error_type == OPERATION_ERROR)
      {
        OperationError *error = publish_result.GetOperationError();
        if (error->GetMessage().has_value())
        {
          std::cerr << "Greengrass Core responded with an error: " << error->GetMessage().value().c_str();
          exit(-1);
        }
      }
      else
      {
        std::cerr << "Attempting to receive the response from the server failed with error code %s\n"
                  << publish_result.GetRpcError().StatusToString().c_str();
        exit(-1);
      }
    }
      std::cout << "Function Complete" << std::endl;
  }
};

class IpcClientLifecycleHandler : public ConnectionLifecycleHandler
{
  void OnConnectCallback() override
  {
    std::cout << "Connected to IPC service." << std::endl;
    // Handle connection to IPC service.
  }

  void OnDisconnectCallback(RpcError error) override
  {
    std::cout << "Disconnected from IPC service. Error: " << std::endl;
    // Handle disconnection from IPC service.
  }

  bool OnErrorCallback(RpcError error) override
  {
    std::cout << "IPC service connection error: " << std::endl;
    // Handle IPC service connection error.
    return true;
  }
};

int main()
{
  std::shared_ptr<IpcClientLifecycleHandler>
      ipc_lifecycle_handler_; ///< Pointer to an instance of IpcClientLifecycleHandler for managing IPC client
                              ///< lifecycle.

  std::unique_ptr<Aws::Crt::ApiHandle> apiHandle_; ///< Pointer to an instance of ApiHandle for API interactions.
  std::unique_ptr<Aws::Crt::Io::EventLoopGroup>
      event_loop_group_; ///< Pointer to an instance of EventLoopGroup for managing event loops.
  std::unique_ptr<Aws::Crt::Io::DefaultHostResolver>
      socket_resolver_; ///< Pointer to an instance of DefaultHostResolver for socket resolution.
  std::unique_ptr<Aws::Crt::Io::ClientBootstrap>
      client_bootstrap_; ///< Pointer to an instance of ClientBootstrap for setting up client connections.
  std::shared_ptr<Aws::Greengrass::GreengrassCoreIpcClient>
      ipc_client_; ///< Pointer to an instance of GreengrassCoreIpcClient for Greengrass Core IPC communication.

  /// Component update stream handler
  std::shared_ptr<ComponentUpdateHandler> ipc_update_handler;
  std::shared_ptr<Aws::Greengrass::SubscribeToComponentUpdatesOperation> ipc_update_operation;

  // Get the value of the AWS_IOT_THING_NAME environment variable
  const char *awsIotThingName = std::getenv("AWS_IOT_THING_NAME");
  const char *gg_version = std::getenv("GGC_VERSION");
  const char *region = std::getenv("AWS_REGION");
  int32_t timeout = 10; // seconds

  // Check if the environment variable exists
  if (awsIotThingName)
  {
    std::cout << "AWS IoT Thing Name: " << awsIotThingName << std::endl;
    if (gg_version)
      std::cout << "ggc version: " << gg_version << std::endl;
    if (region)
      std::cout << "region : " << region << std::endl;

    // Initialize ApiHandle
    apiHandle_ = std::make_unique<Aws::Crt::ApiHandle>(DefaultAllocator());

    // Initialize EventLoopGroup
    event_loop_group_ = std::make_unique<Aws::Crt::Io::EventLoopGroup>(1);

    // Initialize DefaultHostResolver
    socket_resolver_ = std::make_unique<Aws::Crt::Io::DefaultHostResolver>(*event_loop_group_, 64, 30);

    // Initialize ClientBootstrap
    client_bootstrap_ = std::make_unique<Aws::Crt::Io::ClientBootstrap>(*event_loop_group_, *socket_resolver_);

    // Initialize IpcClientLifecycleHandler
    ipc_lifecycle_handler_ = std::make_shared<IpcClientLifecycleHandler>();

    // Initialize GreengrassCoreIpcClient
    ipc_client_ = std::make_shared<Aws::Greengrass::GreengrassCoreIpcClient>(*client_bootstrap_);

    // Connect to IPC
    // Check IPC connection status
    if (auto connectionStatus = ipc_client_->Connect(*ipc_lifecycle_handler_).get(); !connectionStatus)
    {
      std::cout << "Failed to establish IPC connection: " << connectionStatus.StatusToString();
      return -1;
    }
    else
    {
      // MQTT setup successful, mark as connected
      std::cout << "IPC service connect succeeded." << std::endl;
    }

    // Subscribe to updates
    std::cout << "Subscribing to updates SubscribeToComponentUpdatesRequest\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // SubscribeToConfigurationUpdateRequest
    Aws::Greengrass::SubscribeToComponentUpdatesRequest request;
    std::cout << "ipc_update_handler = std::make_shared\n";

    ipc_update_handler = std::make_shared<ComponentUpdateHandler>();
    std::cout << "ipc_update_handler->AddIPCHandler\n";

    // ipc_update_handler->AddIPCHandler(ipc_client_);
    ipc_update_handler->ipc_client_ = ipc_client_;
    std::cout << "ipc_client_->NewSubscribeToComponentUpdates\n";

    ipc_update_operation = ipc_client_->NewSubscribeToComponentUpdates(ipc_update_handler);
    std::cout << "ipc_update_operation->Activate\n";
    auto activate = ipc_update_operation->Activate(request, nullptr);
    std::cout << "activate.wait\n";
    activate.wait();

    std::cout << "ipc_update_operation->GetResult\n";

    auto responseFuture = ipc_update_operation->GetResult();
    if (responseFuture.wait_for(std::chrono::seconds(timeout)) == std::future_status::timeout)
    {
      std::cout << "Operation timed out while waiting for response from Greengrass Core." << std::endl;
    }
    std::cout << "responseFuture.get\n";

    auto response = responseFuture.get();
    if (response)
    {
      std::cout << "Successfully subscribed component updates: " << std::endl;
    }
    else
    {
      // An error occurred.
      std::cout << "Failed to subscribe component updates: " << std::endl;
      auto errorType = response.GetResultType();
      if (errorType == OPERATION_ERROR)
      {
        auto *error = response.GetOperationError();
        std::cout << "Operation error: " << error->GetMessage().value() << std::endl;
      }
      else
      {
        std::cout << "RPC error: " << response.GetRpcError() << std::endl;
      }
    }

    int i = 0;
    while (true)
    {
      std::cout << "sleep: " << i++ << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(60));
    }
  }
  else
  {
    std::cout << "AWS_IOT_THING_NAME environment variable not set. Please start this application from within Greengrass" << std::endl;
  }

  return 0;
}