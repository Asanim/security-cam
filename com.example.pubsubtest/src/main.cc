#include <aws/crt/Api.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>

#include <chrono>
#include <iostream>
#include <thread>

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
      std::cout << "Received JSON message: " << messageString << std::endl;
      // Handle JSON message.
    } else {
      auto binaryMessage = response->GetBinaryMessage();
      if (binaryMessage.has_value() && binaryMessage.value().GetMessage().has_value()) {
        auto messageBytes = binaryMessage.value().GetMessage().value();
        std::string messageString(messageBytes.begin(), messageBytes.end());
        std::cout << "Received binary message: " << messageString << std::endl;
        // Handle binary message.
      }
    }
  }

  bool OnStreamError(OperationError *error) override {
    std::cout << "Stream error: " << std::endl;
    // Handle error.
    return false;  // Return true to close the stream, false to keep the stream open.
  }

  void OnStreamClosed() override {
    std::cout << "Stream closed." << std::endl;
    // Handle close.
  }
};

class IpcClientLifecycleHandler : public ConnectionLifecycleHandler {
  void OnConnectCallback() override {
    std::cout << "Connected to IPC service." << std::endl;
    // Handle connection to IPC service.
  }

  void OnDisconnectCallback(RpcError error) override {
    std::cout << "Disconnected from IPC service. Error: " << std::endl;
    // Handle disconnection from IPC service.
  }

  bool OnErrorCallback(RpcError error) override {
    std::cout << "IPC service connection error: " << std::endl;
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
  // Get the value of the AWS_IOT_THING_NAME environment variable
  const char *awsIotThingName = std::getenv("AWS_IOT_THING_NAME");
  const char *gg_version = std::getenv("GGC_VERSION");
  const char *region = std::getenv("AWS_REGION");
  const char *ca_path = std::getenv("GG_ROOT_CA_PATH");
  const char *socket_fp = std::getenv("AWS_GG_NUCLEUS_DOMAIN_SOCKET_FILEPATH_FOR_COMPONENT");
  const char *svcuid = std::getenv("SVCUID");
  const char *auth_token = std::getenv("AWS_CONTAINER_AUTHORIZATION_TOKEN");
  const char *cred_uri = std::getenv("AWS_CONTAINER_CREDENTIALS_FULL_URI");

  // Check if the environment variable exists
  if (awsIotThingName) {
    std::cout << "AWS IoT Thing Name: " << awsIotThingName << std::endl;
    std::cout << "ggc version: " << gg_version << std::endl;
    if (region) std::cout << "region : " << region << std::endl;
    if (ca_path) std::cout << "ca_path : " << ca_path << std ::endl;
    if (socket_fp) std::cout << "socket_fp : " << socket_fp << std::endl;
    if (svcuid) std::cout << "svcuid : " << svcuid << std::endl;
    if (auth_token) std::cout << "auth_token : " << auth_token << std::endl;
    if (cred_uri) std::cout << "cred_uri : " << cred_uri << std::endl;

    ApiHandle apiHandle(g_allocator);
    std::cout << "ApiHandle created\n";

    Io::EventLoopGroup eventLoopGroup(1);
    std::cout << "EventLoopGroup created\n";

    Io::DefaultHostResolver socketResolver(eventLoopGroup, 64, 30);
    std::cout << "DefaultHostResolver created\n";

    Io::ClientBootstrap bootstrap(eventLoopGroup, socketResolver);
    std::cout << "ClientBootstrap created\n";

    IpcClientLifecycleHandler ipcLifecycleHandler;
    std::cout << "IpcClientLifecycleHandler created\n";

    GreengrassCoreIpcClient ipcClient(bootstrap);
    std::cout << "GreengrassCoreIpcClient created\n";

    auto connectionStatus = ipcClient.Connect(ipcLifecycleHandler).get();
    std::cout << "Connect complete\n";

    if (!connectionStatus) {
      std::cout << "Failed to establish IPC connection\n";
      std::cerr << "Failed to establish IPC connection: " << connectionStatus.StatusToString() << std::endl;
      exit(-1);
    }
    std::cout << "IPC connection established\n";
    int i = 0;

    String publishTopic("test/publish");

    // Keep the main thread alive, or the process will exit.
    while (true) {
      std::string publishTopicPayload =
          R"({"message": "Test message payload )" + std::to_string(i) + awsIotThingName + R"("})";
      i++;
      // Publish to topic
      std::cout << "new publish op" << std::endl;
      auto publishOperation = ipcClient.NewPublishToIoTCore();
      std::cout << "new publish req" << std::endl;
      PublishToIoTCoreRequest publishRequest;
      std::cout << "set topic name" << std::endl;
      publishRequest.SetTopicName(publishTopic);
      std::cout << "create payload" << std::endl;
      Vector<uint8_t> payload(publishTopicPayload.begin(), publishTopicPayload.end());
      std::cout << "ste playload and qos" << std::endl;
      publishRequest.SetPayload(payload);
      publishRequest.SetQos(QOS_AT_LEAST_ONCE);

      // Publish
      std::cout << "Attempting to publish to" << publishTopic.c_str() << "topic\n";
      auto requestStatus = publishOperation->Activate(publishRequest).get();
      if (!requestStatus)
        std::cerr << "Failed to publish to " << publishTopic.c_str() << " topic with error "
                  << requestStatus.StatusToString().c_str();

      auto publishResultFuture = publishOperation->GetResult();
      auto publishResult = publishResultFuture.get();
      if (publishResult) {
        std::cout << "Successfully published to " << publishTopic.c_str() << " topic\n";
        auto *response = publishResult.GetOperationResponse();
        (void)response;
      } else {
        auto errorType = publishResult.GetResultType();
        if (errorType == OPERATION_ERROR) {
          OperationError *error = publishResult.GetOperationError();
          if (error->GetMessage().has_value())
            std::cout << "Greengrass Core responded with an error: " << error->GetMessage().value().c_str();
        } else {
          std::cout << "Attempting to receive the response from the server failed with error code %s\n"
                    << publishResult.GetRpcError().StatusToString().c_str();
        }
      }
      std::cout << "sleep" << std::endl;

      std::this_thread::sleep_for(std::chrono::seconds(60));
    }

  } else {
    std::cerr << "AWS_IOT_THING_NAME environment variable not set." << std::endl;
  }

  return 0;
}