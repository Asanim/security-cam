#include <chrono>
#include <iostream>
#include <thread>
#include <aws/greengrass/GreengrassCoreIpcClient.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/crt/Api.h>
#include <aws/crt/mqtt/MqttClient.h>

using namespace Aws::Greengrass;
using namespace Aws::Crt;
using namespace Aws::Utils;

class SubscribeResponseHandler : public SubscribeToTopicStreamHandler
{
public:
  virtual ~SubscribeResponseHandler() {}

private:
  void OnStreamEvent(SubscriptionResponseMessage *response) override
  {
    auto jsonMessage = response->GetJsonMessage();
    if (jsonMessage.has_value() && jsonMessage.value().GetMessage().has_value())
    {
      auto messageString = jsonMessage.value().GetMessage().value().View().WriteReadable();
      std::cout << "Received JSON message: " << messageString << std::endl;
      // Handle JSON message.
    }
    else
    {
      auto binaryMessage = response->GetBinaryMessage();
      if (binaryMessage.has_value() && binaryMessage.value().GetMessage().has_value())
      {
        auto messageBytes = binaryMessage.value().GetMessage().value();
        std::string messageString(messageBytes.begin(), messageBytes.end());
        std::cout << "Received binary message: " << messageString << std::endl;
        // Handle binary message.
      }
    }
  }

  bool OnStreamError(OperationError *error) override
  {
    std::cout << "Stream error: " << std::endl;
    // Handle error.
    return false; // Return true to close the stream, false to keep the stream open.
  }

  void OnStreamClosed() override
  {
    std::cout << "Stream closed." << std::endl;
    // Handle close.
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

class SubscribeUpdatesHandler : public SubscribeToComponentUpdatesStreamHandler
{
public:
  virtual ~SubscribeUpdatesHandler() {}

  // TODO also subscribe to the board io monitor
  bool IsUpdateReady()
  {
    std::cout << "System Update Ready?" << std::endl;

    return true;
  }

private:
  GreengrassCoreIpcClient *ipcClient;

  void OnStreamEvent(ComponentUpdatePolicyEvents *response) override
  {
    try
    {
      // pre update event
      if (response->GetPreUpdateEvent().has_value())
      {
        if (IsUpdateReady())
        {
          deferUpdate(response->GetPreUpdateEvent().value().GetDeploymentId().value());
        }
        else
        {
          acknowledgeUpdate(response->GetPreUpdateEvent().value().GetDeploymentId().value());
        }
      }
      else if (response->GetPostUpdateEvent().has_value())
      {
        // THIS is an AWS defined function... meaning that the update will be considered applied once the install script
        // runs to completion...

        std::cout << "Applied update for deployment" << std::endl;

        // TODO: set the database to update complete?
        // TODO: Run our self checks here!
      }
    }
    catch (const std::exception &e)
    {
      std::cerr << "Exception caught: " << e.what() << std::endl;
    }
  }

  void deferUpdate(Aws::Crt::String deploymentId)
  {
    std::cout << "Deferring deployment: " << deploymentId << std::endl;

    ipcClient->NewDeferComponentUpdate();
  }

  void acknowledgeUpdate(Aws::Crt::String deploymentId)
  {
    std::cout << "Acknowledging deployment: " << deploymentId << std::endl;
  }

  bool OnStreamError(OperationError *error) override
  {
    std::cerr << "Operation error" << error->GetMessage().value() << std::endl;
    // Handle error.
    return false; // Return true to close stream, false to keep stream open.
  }

  bool OnStreamError(ServiceError *error) override
  {
    std::cerr << "Operation error" << error->GetMessage().value() << std::endl;

    // Handle error.
    return false; // Return true to close stream, false to keep stream open.
  }

  bool OnStreamError(ResourceNotFoundError *error) override
  {
    std::cerr << "Operation error" << error->GetMessage().value() << std::endl;

    // Handle error.
    return false; // Return true to close stream, false to keep stream open.
  }

  void OnStreamClosed() override
  {
    std::cerr << "Operation error" << std::endl;

    // Handle close.
  }
};

class IpcClient
{
public:
  IpcClient()
  {
    InitializeEnvironmentVariables();
    InitializeAwsComponents();
  }

  void Run()
  {
    ConnectToIpc();
    StartPublishingLoop();
  }

private:
  const char *awsIotThingName;
  const char *ggVersion;
  const char *region;
  const char *caPath;
  const char *socketFp;
  const char *svcuid;
  const char *authToken;
  const char *credUri;

  ApiHandle apiHandle{g_allocator};
  Io::EventLoopGroup eventLoopGroup{1};
  Io::DefaultHostResolver socketResolver{eventLoopGroup, 64, 30};
  Io::ClientBootstrap bootstrap{eventLoopGroup, socketResolver};
  GreengrassCoreIpcClient ipcClient{bootstrap};

  void InitializeEnvironmentVariables()
  {
    awsIotThingName = std::getenv("AWS_IOT_THING_NAME");
    ggVersion = std::getenv("GGC_VERSION");
    region = std::getenv("AWS_REGION");
    caPath = std::getenv("GG_ROOT_CA_PATH");
    socketFp = std::getenv("AWS_GG_NUCLEUS_DOMAIN_SOCKET_FILEPATH_FOR_COMPONENT");
    svcuid = std::getenv("SVCUID");
    authToken = std::getenv("AWS_CONTAINER_AUTHORIZATION_TOKEN");
    credUri = std::getenv("AWS_CONTAINER_CREDENTIALS_FULL_URI");

    if (!awsIotThingName)
    {
      throw std::runtime_error("AWS_IOT_THING_NAME environment variable not set.");
    }
  }

  void InitializeAwsComponents()
  {
    std::cout << "AWS Components Initialized" << std::endl;
  }

  void ConnectToIpc()
  {
    IpcClientLifecycleHandler ipcLifecycleHandler;
    auto connectionStatus = ipcClient.Connect(ipcLifecycleHandler).get();

    if (!connectionStatus)
    {
      std::cerr << "Failed to establish IPC connection: " << connectionStatus.StatusToString() << std::endl;
      throw std::runtime_error("IPC connection failed");
    }
    std::cout << "IPC connection established" << std::endl;
  }

  void StartPublishingLoop()
  {
    int i = 0;
    String publishTopic("test/publish");

    while (true)
    {
      std::string payload = R"({"message": "Test message payload )" + std::to_string(i++) + awsIotThingName + R"("})";
      PublishToIoTCore(publishTopic, payload);
      std::this_thread::sleep_for(std::chrono::seconds(60));
    }
  }

  void PublishToIoTCore(const String &topic, const std::string &payload)
  {
    auto publishOperation = ipcClient.NewPublishToIoTCore();
    PublishToIoTCoreRequest request;
    request.SetTopicName(topic);
    request.SetPayload({payload.begin(), payload.end()});
    request.SetQos(QOS_AT_LEAST_ONCE);

    auto requestStatus = publishOperation->Activate(request).get();
    if (!requestStatus)
    {
      std::cerr << "Failed to publish to " << topic.c_str() << " topic: " << requestStatus.StatusToString().c_str() << std::endl;
      return;
    }

    auto result = publishOperation->GetResult().get();
    if (result)
    {
      std::cout << "Successfully published to " << topic.c_str() << " topic" << std::endl;
    }
    else
    {
      std::cerr << "Failed to receive server response" << std::endl;
    }
  }
};