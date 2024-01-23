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
};

#endif // LOGGER_H
