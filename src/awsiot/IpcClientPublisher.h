#ifndef IPC_CLIENT_PUBLISHER_H
#define IPC_CLIENT_PUBLISHER_H

#include <aws/greengrass/GreengrassCoreIpcClient.h>
#include <iostream>
#include <thread>
#include <chrono>

struct IotMessage {
    std::string &publishTopic;
    std::string &payload;
}

class IpcClientPublisher {
public:
    IpcClientPublisher();
    ~IpcClientPublisher();

    void StartPublishing();

    const std::string& GetAwsIotThingName() const { return awsIotThingName_; }
    const std::string& GetGgVersion() const { return gg_version_; }
    const std::string& GetRegion() const { return region_; }
    const std::string& GetCaPath() const { return ca_path_; }
    const std::string& GetSocketFilePath() const { return socket_fp_; }
    const std::string& GetSvcUid() const { return svcuid_; }
    const std::string& GetAuthToken() const { return auth_token_; }
    const std::string& GetCredUri() const { return cred_uri_; }

private:
    const std::string awsIotThingName_ = std::getenv("AWS_IOT_THING_NAME") ? std::getenv("AWS_IOT_THING_NAME") : "";
    const std::string gg_version_ = std::getenv("GGC_VERSION") ? std::getenv("GGC_VERSION") : "";
    const std::string region_ = std::getenv("AWS_REGION") ? std::getenv("AWS_REGION") : "";
    const std::string ca_path_ = std::getenv("GG_ROOT_CA_PATH") ? std::getenv("GG_ROOT_CA_PATH") : "";
    const std::string socket_fp_ = std::getenv("AWS_GG_NUCLEUS_DOMAIN_SOCKET_FILEPATH_FOR_COMPONENT") ?
                                    std::getenv("AWS_GG_NUCLEUS_DOMAIN_SOCKET_FILEPATH_FOR_COMPONENT") : "";
    const std::string svcuid_ = std::getenv("SVCUID") ? std::getenv("SVCUID") : "";
    const std::string auth_token_ = std::getenv("AWS_CONTAINER_AUTHORIZATION_TOKEN") ?
                                    std::getenv("AWS_CONTAINER_AUTHORIZATION_TOKEN") : "";
    const std::string cred_uri_ = std::getenv("AWS_CONTAINER_CREDENTIALS_FULL_URI") ?
                                  std::getenv("AWS_CONTAINER_CREDENTIALS_FULL_URI") : "";


    ApiHandle apiHandle_;
    Io::EventLoopGroup eventLoopGroup_;
    Io::DefaultHostResolver socketResolver_;
    Io::ClientBootstrap bootstrap_;
    IpcClientLifecycleHandler ipcLifecycleHandler_;
    GreengrassCoreIpcClient ipcClient_;
    bool isInitialized_;


    GreengrassCoreIpcClient ipcClient;
    String publishTopic;
    std::thread publishingThread;
    bool stopPublishing;
    int sequenceNumber;

    void PublishMessage(const std::string &payload);

    static void PublishThreadFunction(IpcClientPublisher *publisher);
};

#endif // IPC_CLIENT_PUBLISHER_H
