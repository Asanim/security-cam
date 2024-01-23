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
 *  \file Logger.h
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono>
#include <string>
#include <algorithm>
#include <iostream>

class Logger {
public:
    Logger();
    ~Logger();

    void openLogger();
    void logMemory();
    void readFromFile(const char *filename);

private:
    std::ofstream logFile;
    std::string getTimestamp();

    // system parameter file paths to be logged
    const char *filepaths[7] = {
        "/sys/devices/virtual/thermal/thermal_zone1/temp",
        "/sys/devices/virtual/thermal/thermal_zone0/temp",
        "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq",
        "/sys/devices/system/cpu/cpu1/cpufreq/cpuinfo_cur_freq",
        "/sys/devices/system/cpu/cpu2/cpufreq/cpuinfo_cur_freq",
        "/sys/devices/system/cpu/cpu3/cpufreq/cpuinfo_cur_freq",
        "/sys/devices/platform/ffbc0000.npu/devfreq/ffbc0000.npu/cur_freq"};
};

#endif // LOGGER_H
