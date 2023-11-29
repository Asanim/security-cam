#include "sqlite_database.h"
#include <iostream>

SQLiteDatabase::SQLiteDatabase(const std::string& db_path) {
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        exit(EXIT_FAILURE);
    }
    initDatabase();
}

SQLiteDatabase::~SQLiteDatabase() {
    sqlite3_close(db);
}

void SQLiteDatabase::initDatabase() {
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
    int rc = sqlite3_exec(db, createDetectionTableSQL, nullptr, nullptr, &errMsg);
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

void SQLiteDatabase::saveDetectionResult(const DetectionResult& result) {
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

void SQLiteDatabase::saveSettings(const Settings& settings) {
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

Settings SQLiteDatabase::loadSettings() {
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
