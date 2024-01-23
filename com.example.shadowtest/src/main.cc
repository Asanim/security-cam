#include <aws/crt/Api.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>

#include <chrono>
#include <iostream>
#include <thread>

using namespace Aws::Crt;
using namespace Aws::Greengrass;


#include <iostream>
#include <awsiot/greengrasscoreipc/GreengrassCoreIpcClient.h>
#include <awsiot/greengrasscoreipc/model/GetThingShadowRequest.h>

const int TIMEOUT = 10;

std::vector<uint8_t> sampleGetThingShadowRequest(const std::string& thingName, const std::string& shadowName) {
    try {
        // Set up IPC client to connect to the IPC server
        auto ipcClient = awsiot::greengrass::coreipc::GreengrassCoreIpcClient::Create();

        // Create the GetThingShadow request
        auto getThingShadowRequest = awsiot::greengrass::coreipc::model::GetThingShadowRequest();
        getThingShadowRequest.SetThingName(thingName);
        getThingShadowRequest.SetShadowName(shadowName);

        // Retrieve the GetThingShadow response after sending the request to the IPC server
        auto getThingShadowOperation = ipcClient->NewGetThingShadow();
        getThingShadowOperation->Activate(getThingShadowRequest);
        auto getThingShadowResultFuture = getThingShadowOperation->GetResult();
        auto getThingShadowResult = getThingShadowResultFuture.get();
        
        if (getThingShadowResult) {
            std::cout << "Successfully retrieved thing shadow for " << thingName << "\n";
            auto response = getThingShadowResult.GetOperationResponse();
            return response->GetPayload();
        } else {
            auto errorType = getThingShadowResult.GetResultType();
            if (errorType == awsiot::greengrass::coreipc::model::OperationErrorType::OPERATION_ERROR) {
                auto error = getThingShadowResult.GetOperationError();
                if (error->GetMessage().has_value())
                    std::cerr << "Greengrass Core responded with an error: " << error->GetMessage().value().c_str();
            } else {
                std::cerr << "Attempting to receive the response from the server failed with error code "
                          << getThingShadowResult.GetRpcError().StatusToString().c_str() << "\n";
            }
        }
    } catch (awsiot::greengrass::coreipc::model::InvalidArgumentsError& e) {
        // Add error handling
        std::cerr << "Invalid arguments error: " << e.what() << "\n";
        // ...
    } catch (std::exception& e) {
        // Add additional exception handling
        std::cerr << "Exception occurred: " << e.what() << "\n";
        // ...
    }

    // Handle other errors (ResourceNotFoundError, UnauthorizedError, ServiceError) as needed
    // ...

    return {}; // Return an empty vector in case of error
}



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

  } else {
    std::cerr << "AWS_IOT_THING_NAME environment variable not set." << std::endl;
  }

  return 0;
}