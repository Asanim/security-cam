#include "IpcClientLifecycleHandler.h"

void IpcClientLifecycleHandler::OnConnectCallback() {
    std::cout << "Connected to IPC service." << std::endl;
    // Handle connection to IPC service.
}

void IpcClientLifecycleHandler::OnDisconnectCallback(RpcError error) {
    std::cout << "Disconnected from IPC service. Error: " << std::endl;
    // Handle disconnection from IPC service.
}

bool IpcClientLifecycleHandler::OnErrorCallback(RpcError error) {
    std::cout << "IPC service connection error: " << std::endl;
    // Handle IPC service connection error.
    return true;
}
