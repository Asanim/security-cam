#include "rest_api_server.h"
#include <pistache/http.h>
#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <pistache/net.h>
#include <pistache/serializer/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <sstream>

RestApiServer::RestApiServer(Pistache::Address addr, SQLiteDatabase& db)
    : httpEndpoint(std::make_shared<Pistache::Http::Endpoint>(addr)), db(db) {}

void RestApiServer::init(size_t threads) {
    auto opts = Pistache::Http::Endpoint::options().threads(static_cast<int>(threads));
    httpEndpoint->init(opts);
    setupRoutes();
}

void RestApiServer::start() {
    httpEndpoint->setHandler(router.handler());
    httpEndpoint->serve();
}

void RestApiServer::shutdown() {
    httpEndpoint->shutdown();
}

void RestApiServer::setupRoutes() {
    using namespace Pistache::Rest;

    Routes::Post(router, "/detection_results", Routes::bind(&RestApiServer::handleDetectionResults, this));
    Routes::Get(router, "/settings", Routes::bind(&RestApiServer::handleGetSettings, this));
    Routes::Post(router, "/settings", Routes::bind(&RestApiServer::handleSetSettings, this));
}

void RestApiServer::handleDetectionResults(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    rapidjson::Document doc;
    doc.Parse(request.body().c_str());

    if (!doc.HasMember("class_id") || !doc.HasMember("class_name") || !doc.HasMember("confidence") || !doc.HasMember("bbox")) {
        response.send(Pistache::Http::Code::Bad_Request, "Invalid JSON");
        return;
    }

    DetectionResult result;
    result.class_id = doc["class_id"].GetInt();
    result.class_name = doc["class_name"].GetString();
    result.confidence = doc["confidence"].GetFloat();
    result.bbox = cv::Rect(doc["bbox"]["x"].GetInt(), doc["bbox"]["y"].GetInt(), doc["bbox"]["width"].GetInt(), doc["bbox"]["height"].GetInt());

    db.saveDetectionResult(result);
    response.send(Pistache::Http::Code::Ok, "Detection result saved");
}

void RestApiServer::handleGetSettings(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    Settings settings = db.loadSettings();

    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    doc.AddMember("ai_model_name", rapidjson::Value(settings.ai_model_name.c_str(), allocator), allocator);
    doc.AddMember("camera_source", rapidjson::Value(settings.camera_source.c_str(), allocator), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    response.send(Pistache::Http::Code::Ok, buffer.GetString());
}

void RestApiServer::handleSetSettings(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    rapidjson::Document doc;
    doc.Parse(request.body().c_str());

    if (!doc.HasMember("ai_model_name") || !doc.HasMember("camera_source")) {
        response.send(Pistache::Http::Code::Bad_Request, "Invalid JSON");
        return;
    }

    Settings settings;
    settings.ai_model_name = doc["ai_model_name"].GetString();
    settings.camera_source = doc["camera_source"].GetString();

    db.saveSettings(settings);
    response.send(Pistache::Http::Code::Ok, "Settings saved");
}
