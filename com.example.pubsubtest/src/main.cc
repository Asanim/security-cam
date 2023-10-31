// #include <aws/crt/Api.h>
// #include <aws/greengrass/GreengrassCoreIpcClient.h>
// #include <iostream>
// #include <thread>
// #include <chrono>

// using namespace Aws::Crt;
// using namespace Aws::Greengrass;

// class SubscribeResponseHandler : public SubscribeToTopicStreamHandler {
//  public:
//   virtual ~SubscribeResponseHandler() {}

//  private:
//   void OnStreamEvent(SubscriptionResponseMessage *response) override {
//     auto jsonMessage = response->GetJsonMessage();
//     if (jsonMessage.has_value() && jsonMessage.value().GetMessage().has_value()) {
//       auto messageString = jsonMessage.value().GetMessage().value().View().WriteReadable();
//       // Handle JSON message.
//     } else {
//       auto binaryMessage = response->GetBinaryMessage();
//       if (binaryMessage.has_value() && binaryMessage.value().GetMessage().has_value()) {
//         auto messageBytes = binaryMessage.value().GetMessage().value();
//         std::string messageString(messageBytes.begin(), messageBytes.end());
//         // Handle binary message.
//       }
//     }
//   }

//   bool OnStreamError(OperationError *error) override {
//     // Handle error.
//     return false;  // Return true to close stream, false to keep stream open.
//   }

//   void OnStreamClosed() override {
//     // Handle close.
//   }
// };

// class IpcClientLifecycleHandler : public ConnectionLifecycleHandler {
//   void OnConnectCallback() override {
//     // Handle connection to IPC service.
//   }

//   void OnDisconnectCallback(RpcError error) override {
//     // Handle disconnection from IPC service.
//   }

//   bool OnErrorCallback(RpcError error) override {
//     // Handle IPC service connection error.
//     return true;
//   }
// };

// ///
// /// \brief
// ///
// ///
// class SubscribeUpdatesHandler : public SubscribeToComponentUpdatesStreamHandler {
//  public:
//   virtual ~SubscribeUpdatesHandler() {}

//   // TODO also subscribe to the board io monitor
//   bool IsUpdateReady() {
//     std::cout << "System Update Ready?" << std::endl;

//     return true;
//   }

//  private:
//   GreengrassCoreIpcClient *ipcClient;

//   void OnStreamEvent(ComponentUpdatePolicyEvents *response) override {
//     try {
//       // pre update event
//       if (response->GetPreUpdateEvent().has_value()) {
//         if (IsUpdateReady()) {
//           deferUpdate(response->GetPreUpdateEvent().value().GetDeploymentId().value());
//         } else {
//           acknowledgeUpdate(response->GetPreUpdateEvent().value().GetDeploymentId().value());
//         }
//       } else if (response->GetPostUpdateEvent().has_value()) {
//         // THIS is an AWS defined function... meaning that the update will be considered applied once the install script
//         // runs to completion...

//         std::cout << "Applied update for deployment" << std::endl;

//         // TODO: set the database to update complete?
//         // TODO: Run our self checks here!
//       }
//     } catch (const std::exception &e) {
//       std::cerr << "Exception caught: " << e.what() << std::endl;
//     }
//   }

//   void deferUpdate(Aws::Crt::String deploymentId) {
//     std::cout << "Deferring deployment: " << deploymentId << std::endl;

//     ipcClient->NewDeferComponentUpdate();
//   }

//   void acknowledgeUpdate(Aws::Crt::String deploymentId) {
//     std::cout << "Acknowledging deployment: " << deploymentId << std::endl;
//   }

//   bool OnStreamError(OperationError *error) override {
//     std::cerr << "Operation error" << error->GetMessage().value() << std::endl;
//     // Handle error.
//     return false;  // Return true to close stream, false to keep stream open.
//   }

//   bool OnStreamError(ServiceError *error) override {
//     std::cerr << "Operation error" << error->GetMessage().value() << std::endl;

//     // Handle error.
//     return false;  // Return true to close stream, false to keep stream open.
//   }

//   bool OnStreamError(ResourceNotFoundError *error) override {
//     std::cerr << "Operation error" << error->GetMessage().value() << std::endl;

//     // Handle error.
//     return false;  // Return true to close stream, false to keep stream open.
//   }

//   void OnStreamClosed() override {
//     std::cerr << "Operation error" << std::endl;

//     // Handle close.
//   }
// };

// int main() {
//   ApiHandle apiHandle(g_allocator);
//   Io::EventLoopGroup eventLoopGroup(1);
//   Io::DefaultHostResolver socketResolver(eventLoopGroup, 64, 30);
//   Io::ClientBootstrap bootstrap(eventLoopGroup, socketResolver);
//   IpcClientLifecycleHandler ipcLifecycleHandler;
//   GreengrassCoreIpcClient ipcClient(bootstrap);
//   auto connectionStatus = ipcClient.Connect(ipcLifecycleHandler).get();
//   if (!connectionStatus) {
//     std::cerr << "Failed to establish IPC connection: " << connectionStatus.StatusToString() << std::endl;
//     exit(-1);
//   }

//   String subscribeTopic("test/subscribe");
//   String publishTopic("test/publish");
//   String publishTopicPayload("Test message payload");
//   int timeout = 10;

//   SubscribeToTopicRequest request;
//   request.SetTopic(subscribeTopic);

//   // SubscribeResponseHandler streamHandler;
//   auto streamHandler = MakeShared<SubscribeResponseHandler>(DefaultAllocator());
//   auto operation = ipcClient.NewSubscribeToTopic(streamHandler);

//   auto activate = operation->Activate(request, nullptr);

//   // TODO subscribe to component updates
//   auto updatesHandler = MakeShared<SubscribeUpdatesHandler>(DefaultAllocator());
//   ipcClient.NewSubscribeToComponentUpdates(updatesHandler);

//   activate.wait();
  
//   // Publish to topic 
//       // Publish to the same topic that is currently subscribed to.
//     auto publishOperation = ipcClient.NewPublishToIoTCore();
//     PublishToIoTCoreRequest publishRequest;
//     publishRequest.SetTopicName(publishTopic);
//     Vector<uint8_t> payload(publishTopicPayload.begin(), publishTopicPayload.end());
//     publishRequest.SetPayload(payload);
//     publishRequest.SetQos(QOS_AT_LEAST_ONCE);

//   // Keep the main thread alive, or the process will exit.
//   while (true) {
//     // Subscribe
//     auto responseFuture = operation->GetResult();
//     if (responseFuture.wait_for(std::chrono::seconds(timeout)) == std::future_status::timeout) {
//       std::cerr << "Operation timed out while waiting for response from Greengrass Core." << std::endl;
//       exit(-1);
//     }

//     auto response = responseFuture.get();
//     if (!response) {
//       // Handle error.
//       auto errorType = response.GetResultType();
//       if (errorType == OPERATION_ERROR) {
//         auto *error = response.GetOperationError();
//         (void)error;
//         // Handle operation error.
//       } else {
//         // Handle RPC error.
//       }
//       exit(-1);
//     }

//     // Publish
//     std::cout << "Attempting to publish to" << publishTopic.c_str() << "topic\n";
//     auto requestStatus = publishOperation->Activate(publishRequest).get();
//     if (!requestStatus)
//       std::cerr << "Failed to publish to " << publishTopic.c_str() << " topic with error "<< requestStatus.StatusToString().c_str();

//     auto publishResultFuture = publishOperation->GetResult();
//     auto publishResult = publishResultFuture.get();
//     if (publishResult)
//     {
//         std::cout << "Successfully published to "<< publishTopic.c_str() <<" topic\n";
//         auto *response = publishResult.GetOperationResponse();
//         (void)response;
//     } else {
//         auto errorType = publishResult.GetResultType();
//         if (errorType == OPERATION_ERROR)
//         {
//             OperationError *error = publishResult.GetOperationError();
//             if (error->GetMessage().has_value())
//                 std::cout << "Greengrass Core responded with an error: " << error->GetMessage().value().c_str();
//         }
//         else
//         {
//           std::cout << "Attempting to receive the response from the server failed with error code %s\n" << publishResult.GetRpcError().StatusToString().c_str();
//         }
//     }

//     std::this_thread::sleep_for(std::chrono::seconds(60));
//   }

//   operation->Close();
//   return 0;
// }

#include <iostream>

#include <aws/crt/Api.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>

using namespace Aws::Crt;
using namespace Aws::Greengrass;

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

int main() {
    std::cout << "Starting component..."; 
    ApiHandle apiHandle(g_allocator);
    Io::EventLoopGroup eventLoopGroup(1);
    Io::DefaultHostResolver socketResolver(eventLoopGroup, 64, 30);
    Io::ClientBootstrap bootstrap(eventLoopGroup, socketResolver);
    std::cout << "Setting up lifecycle"; 
    IpcClientLifecycleHandler ipcLifecycleHandler;
    GreengrassCoreIpcClient ipcClient(bootstrap);
    auto connectionStatus = ipcClient.Connect(ipcLifecycleHandler).get();
    if (!connectionStatus) {
        std::cerr << "Failed to establish IPC connection: " << connectionStatus.StatusToString() << std::endl;
        exit(-1);
    }

    String message("Hello, World!");
    String topic("my/topic");
    QOS qos = QOS_AT_MOST_ONCE;
    int timeout = 10;
    std::cout << "creating request"; 

    PublishToIoTCoreRequest request;
    Vector<uint8_t> messageData({message.begin(), message.end()});
    request.SetTopicName(topic);
    request.SetPayload(messageData);
    request.SetQos(qos);
    std::cout << "publishing to iot core"; 

    auto operation = ipcClient.NewPublishToIoTCore();
    auto activate = operation->Activate(request, nullptr);
    activate.wait();

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
    }

    return 0;
}
