#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <json/json.h>
#include <aws/core/Aws.h>
#include <aws/iotshadow/IoTShadowClient.h>
#include <aws/crt/mqtt/MqttClient.h>
#include <aws/crt/Api.h>
#include <exception>

// Define an enum for the update source
enum class DeviceUpdateSource {
    STARTUP,
    UPDATED_BY_CLOUD_SHADOW,
    UPDATED_BY_LOCAL
};

// Global variables
int CURRENT_NUMBER = 0;
DeviceUpdateSource CURRENT_SOURCE_OF_CHANGE = DeviceUpdateSource::STARTUP;

// AWS IoT parameters
const std::string THING_NAME = "PiWithSenseHat";
const std::string SHADOW_NAME = "NumberLEDNamedShadow";
const std::string JOYSTICK_TOPIC = "ipc/joystick";

Aws::IoTShadow::IoTShadowClient *shadowClient;

// Function prototypes
void updateDeviceAndCloudShadow(int newNumber, DeviceUpdateSource newStatus);
void checkCloudShadowAndUpdateDevice();
void subscribeToJoystickEvents();

void updateDeviceAndCloudShadow(int newNumber, DeviceUpdateSource newStatus) {
    // Update the device state
    CURRENT_NUMBER = newNumber;
    CURRENT_SOURCE_OF_CHANGE = newStatus;

    // Update the shadow state
    Json::Value currentState;
    currentState["state"]["reported"]["status"] = (newStatus == DeviceUpdateSource::STARTUP)
                                                      ? "device is initialized with default value"
                                                      : (newStatus == DeviceUpdateSource::UPDATED_BY_CLOUD_SHADOW)
                                                            ? "device is updated by shadow"
                                                            : "device is updated by local";
    currentState["state"]["reported"]["number"] = CURRENT_NUMBER;

    // If the update was local, update the desired state as well
    if (newStatus == DeviceUpdateSource::UPDATED_BY_LOCAL) {
        currentState["state"]["desired"]["number"] = CURRENT_NUMBER;
    }

    // Serialize the JSON state
    Json::StreamWriterBuilder writer;
    std::string payload = Json::writeString(writer, currentState);

    try {
        // Update the shadow
        Aws::IoTShadow::Model::UpdateShadowRequest request;
        request.WithThingName(THING_NAME).WithShadowName(SHADOW_NAME).WithPayload(payload);
        auto result = shadowClient->UpdateShadow(request);

        std::cout << "Shadow updated successfully: " << result.GetResult().GetPayload() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Failed to update shadow: " << e.what() << std::endl;
    }
}

void checkCloudShadowAndUpdateDevice() {
    try {
        // Get the shadow state
        Aws::IoTShadow::Model::GetShadowRequest request;
        request.WithThingName(THING_NAME).WithShadowName(SHADOW_NAME);
        auto result = shadowClient->GetShadow(request);

        // Parse the JSON payload
        Json::CharReaderBuilder reader;
        Json::Value shadowJson;
        std::string errors;
        std::istringstream stream(result.GetResult().GetPayload());
        if (!Json::parseFromStream(reader, stream, &shadowJson, &errors)) {
            throw std::runtime_error("Failed to parse shadow payload: " + errors);
        }

        // Check if there's a desired state to update
        if (shadowJson["state"].isMember("desired") &&
            shadowJson["state"]["desired"].isMember("number")) {
            int desiredNumber = shadowJson["state"]["desired"]["number"].asInt();
            if (CURRENT_NUMBER != desiredNumber) {
                updateDeviceAndCloudShadow(desiredNumber, DeviceUpdateSource::UPDATED_BY_CLOUD_SHADOW);
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Failed to get shadow: " << e.what() << std::endl;
    }
}

void onJoystickEvent(const std::string &message) {
    try {
        Json::CharReaderBuilder reader;
        Json::Value payload;
        std::string errors;
        std::istringstream stream(message);
        if (!Json::parseFromStream(reader, stream, &payload, &errors)) {
            throw std::runtime_error("Failed to parse joystick message: " + errors);
        }

        std::string direction = payload["direction"].asString();
        std::string action = payload["action"].asString();

        if (action != "pressed") {
            return;
        }

        int newNumber = CURRENT_NUMBER;

        if (direction == "up") {
            newNumber = std::min(CURRENT_NUMBER + 1, 9);
        } else if (direction == "down") {
            newNumber = std::max(CURRENT_NUMBER - 1, 0);
        }

        updateDeviceAndCloudShadow(newNumber, DeviceUpdateSource::UPDATED_BY_LOCAL);
    } catch (const std::exception &e) {
        std::cerr << "Failed to handle joystick event: " << e.what() << std::endl;
    }
}

void subscribeToJoystickEvents() {
    std::cout << "Subscribed to joystick events." << std::endl;
}

int main() {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        Aws::Client::ClientConfiguration config;
        shadowClient = new Aws::IoTShadow::IoTShadowClient(config);

        // Report initial state
        updateDeviceAndCloudShadow(CURRENT_NUMBER, CURRENT_SOURCE_OF_CHANGE);

        // Subscribe to joystick events
        subscribeToJoystickEvents();

        // Periodically check the cloud shadow
        while (true) {
            checkCloudShadowAndUpdateDevice();
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }

        delete shadowClient;
    }
    Aws::ShutdownAPI(options);
    return 0;
}
