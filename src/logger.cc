/* 
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 *  
 *  \file logger.cc
 */

#include "logger.h"

Logger::Logger() {
    // Constructor
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::openLogger() {
    std::string filename;
    std::stringstream ss;
    std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    std::string timeStr = std::ctime(&currentTime);
    std::replace(timeStr.begin(), timeStr.end(), ' ', '_');
    std::replace(timeStr.begin(), timeStr.end(), '\n', '_');

    ss << "/media/firefly/0356bd29-ed3d-4b5d-921f-a7a8362599df/detected_logs/"
       << "log_" << timeStr << ".txt";
    ss >> filename;

    logFile.open(filename);
}

void Logger::logMemory() {
    int memFree = 0;
    int memTotal = 1;
    int memPercent = 0;

    std::ifstream memInfoFile("/proc/meminfo");
    memInfoFile >> memTotal >> memFree;
    memInfoFile.close();

    memPercent = (int)(((float)memFree / memTotal) * 100);

    logFile << memPercent << ", " << memFree << ", " << memTotal << ", ";
}

void Logger::readFromFile(const char *filename) {
    std::string val;

    std::ifstream inputFile;
    inputFile.open(filename);

    if (!inputFile) {
        std::cout << "File not found: " << filename << "\n";
        return;
    }

    inputFile >> val;
    inputFile.close();

    logFile << val << ", ";
}

std::string Logger::getTimestamp() {
    std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string timeStr = std::ctime(&currentTime);
    std::replace(timeStr.begin(), timeStr.end(), ' ', '_');
    std::replace(timeStr.begin(), timeStr.end(), '\n', '_');
    return timeStr;
}
