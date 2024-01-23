#include "IpcClientPublisher.h"

IpcClientPublisher::IpcClientPublisher() {

    IpcClientPublisher::InitAWSIoT();
}

IpcClientPublisher::~IpcClientPublisher() {
    stopPublishing = true;
    if (publishingThread.joinable()) {
        publishingThread.join();
    }
}

void IpcClientPublisher::StartPublishing() {
    publishingThread = std::thread(PublishThreadFunction, this);
}

void IpcClientPublisher::InitAWSIoT() {
    // Check if already initialized
    if (isInitialized_) {
        std::cerr << "AWS IoT has already been initialized\n";
        return;
    }

    // Initialization code
    apiHandle_ = ApiHandle(g_allocator);
    std::cout << "ApiHandle created\n";

    eventLoopGroup_ = Io::EventLoopGroup(1);
    std::cout << "EventLoopGroup created\n";

    socketResolver_ = Io::DefaultHostResolver(eventLoopGroup_, 64, 30);
    std::cout << "DefaultHostResolver created\n";

    bootstrap_ = Io::ClientBootstrap(eventLoopGroup_, socketResolver_);
    std::cout << "ClientBootstrap created\n";

    ipcClient_ = GreengrassCoreIpcClient(bootstrap_);
    std::cout << "GreengrassCoreIpcClient created\n";

    auto connectionStatus = ipcClient_.Connect(ipcLifecycleHandler_).get();
    std::cout << "Connect complete\n";

    if (!connectionStatus) {
        std::cout << "Failed to establish IPC connection\n";
        std::cerr << "Failed to establish IPC connection: " << connectionStatus.StatusToString() << std::endl;
        exit(-1);
    }

    std::cout << "IPC connection established\n";
    isInitialized_ = true;
}

void IpcClientPublisher::SetEnvVariables() {
  // Check if the environment variable exists
  if (awsIotThingName_) {

  }
}


void IpcClientPublisher::PublishMessage(const std::string &payload) {
    auto publishOperation = ipcClient_.NewPublishToIoTCore();
    PublishToIoTCoreRequest publishRequest;
    publishRequest.SetTopicName(publishTopic);
    Vector<uint8_t> payloadBytes(payload.begin(), payload.end());
    publishRequest.SetPayload(payloadBytes);
    publishRequest.SetQos(QOS_AT_LEAST_ONCE);

    auto requestStatus = publishOperation->Activate(publishRequest).get();
    if (!requestStatus) {
        std::cerr << "Failed to publish message with error: " << requestStatus.StatusToString().c_str() << std::endl;
    } else {
        auto publishResultFuture = publishOperation->GetResult();
        auto publishResult = publishResultFuture.get();
        if (publishResult) {
            std::cout << "Successfully published message\n";
        } else {
            std::cerr << "Failed to publish message with error: "
                      << publishResult.GetRpcError().StatusToString().c_str() << std::endl;
        }
    }
}

void IpcClientPublisher::PublishThreadFunction(IpcClientPublisher *publisher) {
    while (!publisher->stopPublishing) {
        std::string messagePayload =
            R"({"message": "Test message payload " + std::to_string(publisher->sequenceNumber++) + "})";
        publisher->PublishMessage(messagePayload);

        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}
