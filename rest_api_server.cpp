#include "rest_api_server.h"
#include <pistache/http.h>
#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <pistache/net.h>
#include <pistache/serializer/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <sqlite3.h>
#include <iostream>
#include <sstream>

RestApiServer::RestApiServer(Pistache::Address addr)
    : httpEndpoint(std::make_shared<Pistache::Http::Endpoint>(addr)) {
    initDatabase();
}

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
    sqlite3_close(db);
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

    saveDetectionResult(result);
    response.send(Pistache::Http::Code::Ok, "Detection result saved");
}

void RestApiServer::handleGetSettings(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    Settings settings = loadSettings();

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

    saveSettings(settings);
    response.send(Pistache::Http::Code::Ok, "Settings saved");
}

void RestApiServer::initDatabase() {
    int rc = sqlite3_open("security_cam.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        exit(EXIT_FAILURE);
    }

    const char* createDetectionTableSQL = R"(
        CREATE TABLE IF NOT EXISTS detection_results (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            class_id INTEGER,
            class_name TEXT,
            confidence REAL,
            bbox_x INTEGER,
            bbox_y INTEGER,
            bbox_width INTEGER,
            bbox_height INTEGER
        );
    )";

    const char* createSettingsTableSQL = R"(
        CREATE TABLE IF NOT EXISTS settings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            ai_model_name TEXT,
            camera_source TEXT
        );
    )";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db, createDetectionTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_exec(db, createSettingsTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        exit(EXIT_FAILURE);
    }
}

void RestApiServer::saveDetectionResult(const DetectionResult& result) {
    const char* insertSQL = R"(
        INSERT INTO detection_results (class_id, class_name, confidence, bbox_x, bbox_y, bbox_width, bbox_height)
        VALUES (?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, result.class_id);
    sqlite3_bind_text(stmt, 2, result.class_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, result.confidence);
    sqlite3_bind_int(stmt, 4, result.bbox.x);
    sqlite3_bind_int(stmt, 5, result.bbox.y);
    sqlite3_bind_int(stmt, 6, result.bbox.width);
    sqlite3_bind_int(stmt, 7, result.bbox.height);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void RestApiServer::saveSettings(const Settings& settings) {
    const char* deleteSQL = "DELETE FROM settings;";
    sqlite3_exec(db, deleteSQL, nullptr, nullptr, nullptr);

    const char* insertSQL = R"(
        INSERT INTO settings (ai_model_name, camera_source)
        VALUES (?, ?);
    )";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, settings.ai_model_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, settings.camera_source.c_str(), -1, SQLITE_STATIC);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

Settings RestApiServer::loadSettings() {
    Settings settings;

    const char* selectSQL = "SELECT ai_model_name, camera_source FROM settings LIMIT 1;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        settings.ai_model_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        settings.camera_source = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    }

    sqlite3_finalize(stmt);
    return settings;
}
