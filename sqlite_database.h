#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

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

class SQLiteDatabase {
public:
    SQLiteDatabase(const std::string& db_path);
    ~SQLiteDatabase();

    void saveDetectionResult(const DetectionResult& result);
    void saveSettings(const Settings& settings);
    Settings loadSettings();

private:
    void initDatabase();

    sqlite3* db;
};
