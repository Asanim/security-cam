#pragma once

#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <pistache/serializer/rapidjson.h>
#include <sqlite3.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

struct DetectionResult {
    int class_id;
    std::string class_name;
    float confidence;
    cv::Rect bbox;
};

struct Settings {
    std::string ai_model_name;
    std::string camera_source;
};

class RestApiServer {
public:
    RestApiServer(Pistache::Address addr);
    void init(size_t threads = 2);
    void start();
    void shutdown();

private:
    void setupRoutes();
    void handleDetectionResults(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void handleSettings(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void handleGetSettings(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void handleSetSettings(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    void initDatabase();
    void saveDetectionResult(const DetectionResult& result);
    void saveSettings(const Settings& settings);
    Settings loadSettings();

    Pistache::Http::Endpoint httpEndpoint;
    Pistache::Rest::Router router;
    sqlite3* db;
};
