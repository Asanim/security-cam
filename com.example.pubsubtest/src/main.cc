#include <aws/crt/Api.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace Aws::Crt;
using namespace Aws::Greengrass;

class SubscribeResponseHandler : public SubscribeToTopicStreamHandler {
 public:
  virtual ~SubscribeResponseHandler() {}

 private:
  void OnStreamEvent(SubscriptionResponseMessage *response) override {
    auto jsonMessage = response->GetJsonMessage();
    if (jsonMessage.has_value() && jsonMessage.value().GetMessage().has_value()) {
      auto messageString = jsonMessage.value().GetMessage().value().View().WriteReadable();
      // Handle JSON message.
    } else {
      auto binaryMessage = response->GetBinaryMessage();
      if (binaryMessage.has_value() && binaryMessage.value().GetMessage().has_value()) {
        auto messageBytes = binaryMessage.value().GetMessage().value();
        std::string messageString(messageBytes.begin(), messageBytes.end());
        // Handle binary message.
      }
    }
  }

  bool OnStreamError(OperationError *error) override {
    // Handle error.
    return false;  // Return true to close stream, false to keep stream open.
  }

  void OnStreamClosed() override {
    // Handle close.
  }
};

class IpcClientLifecycleHandler : public ConnectionLifecycleHandler {
  void OnConnectCallback() override {
    // Handle connection to IPC service.
  }

  void OnDisconnectCallback(RpcError error) override {
    // Handle disconnection from IPC service.
  }

  bool OnErrorCallback(RpcError error) override {
    // Handle IPC service connection error.
    return true;
  }
};

///
/// \brief
///
///
class SubscribeUpdatesHandler : public SubscribeToComponentUpdatesStreamHandler {
 public:
  virtual ~SubscribeUpdatesHandler() {}

  // TODO also subscribe to the board io monitor
  bool IsUpdateReady() {
    std::cout << "System Update Ready?" << std::endl;

    return true;
  }

 private:
  GreengrassCoreIpcClient *ipcClient;

  void OnStreamEvent(ComponentUpdatePolicyEvents *response) override {
    try {
      // pre update event
      if (response->GetPreUpdateEvent().has_value()) {
        if (IsUpdateReady()) {
          deferUpdate(response->GetPreUpdateEvent().value().GetDeploymentId().value());
        } else {
          acknowledgeUpdate(response->GetPreUpdateEvent().value().GetDeploymentId().value());
        }
      } else if (response->GetPostUpdateEvent().has_value()) {
        // THIS is an AWS defined function... meaning that the update will be considered applied once the install script
        // runs to completion...

        std::cout << "Applied update for deployment" << std::endl;

        // TODO: set the database to update complete?
        // TODO: Run our self checks here!
      }
    } catch (const std::exception &e) {
      std::cerr << "Exception caught: " << e.what() << std::endl;
    }
  }

  void deferUpdate(Aws::Crt::String deploymentId) {
    std::cout << "Deferring deployment: " << deploymentId << std::endl;

    ipcClient->NewDeferComponentUpdate();
  }

  void acknowledgeUpdate(Aws::Crt::String deploymentId) {
    std::cout << "Acknowledging deployment: " << deploymentId << std::endl;
  }

  bool OnStreamError(OperationError *error) override {
    std::cerr << "Operation error" << error->GetMessage().value() << std::endl;
    // Handle error.
    return false;  // Return true to close stream, false to keep stream open.
  }

  bool OnStreamError(ServiceError *error) override {
    std::cerr << "Operation error" << error->GetMessage().value() << std::endl;

    // Handle error.
    return false;  // Return true to close stream, false to keep stream open.
  }

  bool OnStreamError(ResourceNotFoundError *error) override {
    std::cerr << "Operation error" << error->GetMessage().value() << std::endl;

    // Handle error.
    return false;  // Return true to close stream, false to keep stream open.
  }

  void OnStreamClosed() override {
    std::cerr << "Operation error" << std::endl;

    // Handle close.
  }
};

int main() {
  ApiHandle apiHandle(g_allocator);
  Io::EventLoopGroup eventLoopGroup(1);
  Io::DefaultHostResolver socketResolver(eventLoopGroup, 64, 30);
  Io::ClientBootstrap bootstrap(eventLoopGroup, socketResolver);
  IpcClientLifecycleHandler ipcLifecycleHandler;
  GreengrassCoreIpcClient ipcClient(bootstrap);
  auto connectionStatus = ipcClient.Connect(ipcLifecycleHandler).get();
  if (!connectionStatus) {
    std::cerr << "Failed to establish IPC connection: " << connectionStatus.StatusToString() << std::endl;
    exit(-1);
  }

  String topic("test_topic");
  int timeout = 10;

  SubscribeToTopicRequest request;
  request.SetTopic(topic);

  // SubscribeResponseHandler streamHandler;
  auto streamHandler = MakeShared<SubscribeResponseHandler>(DefaultAllocator());
  auto operation = ipcClient.NewSubscribeToTopic(streamHandler);

  auto activate = operation->Activate(request, nullptr);

  // TODO subscribe to component updates
  auto updatesHandler = MakeShared<SubscribeUpdatesHandler>(DefaultAllocator());
  ipcClient.NewSubscribeToComponentUpdates(updatesHandler);

  activate.wait();
  

  // Keep the main thread alive, or the process will exit.
  while (true) {
    auto responseFuture = operation->GetResult();
    if (responseFuture.wait_for(std::chrono::seconds(timeout)) == std::future_status::timeout) {
      std::cerr << "Operation timed out while waiting for response from Greengrass Core." << std::endl;
      exit(-1);
    }

    auto response = responseFuture.get();
    if (!response) {
      // Handle error.
      auto errorType = response.GetResultType();
      if (errorType == OPERATION_ERROR) {
        auto *error = response.GetOperationError();
        (void)error;
        // Handle operation error.
      } else {
        // Handle RPC error.
      }
      exit(-1);
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));
  }

  operation->Close();
  return 0;
}