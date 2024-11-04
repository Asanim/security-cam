#include "SubscribeUpdatesHandler.h"

SubscribeUpdatesHandler::~SubscribeUpdatesHandler() {
    // Destructor
}

bool SubscribeUpdatesHandler::IsUpdateReady() {
    std::cout << "System Update Ready?" << std::endl;
    return true;
}

void SubscribeUpdatesHandler::OnStreamEvent(ComponentUpdatePolicyEvents *response) {
    try {
        if (response->GetPreUpdateEvent().has_value()) {
            if (IsUpdateReady()) {
                deferUpdate(response->GetPreUpdateEvent().value().GetDeploymentId().value());
            } else {
                acknowledgeUpdate(response->GetPreUpdateEvent().value().GetDeploymentId().value());
            }
        } else if (response->GetPostUpdateEvent().has_value()) {
            std::cout << "Applied update for deployment" << std::endl;
            // TODO: set the database to update complete?
            // TODO: Run our self checks here!
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

void SubscribeUpdatesHandler::deferUpdate(Aws::Crt::String deploymentId) {
    std::cout << "Deferring deployment: " << deploymentId << std::endl;
    ipcClient->NewDeferComponentUpdate();
}

void SubscribeUpdatesHandler::acknowledgeUpdate(Aws::Crt::String deploymentId) {
    std::cout << "Acknowledging deployment: " << deploymentId << std::endl;
}

bool SubscribeUpdatesHandler::OnStreamError(OperationError *error) {
    std::cerr << "Operation error" << error->GetMessage().value() << std::endl;
    // Handle error.
    return false;  // Return true to close stream, false to keep stream open.
}

bool SubscribeUpdatesHandler::OnStreamError(ServiceError *error) {
    std::cerr << "Operation error" << error->GetMessage().value() << std::endl;
    // Handle error.
    return false;  // Return true to close stream, false to keep stream open.
}

bool SubscribeUpdatesHandler::OnStreamError(ResourceNotFoundError *error) {
    std::cerr << "Operation error" << error->GetMessage().value() << std::endl;
    // Handle error.
    return false;  // Return true to close stream, false to keep stream open.
}

void SubscribeUpdatesHandler::OnStreamClosed() {
    std::cerr << "Operation error" << std::endl;
    // Handle close.
}
