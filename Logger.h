#pragma once
#include <string>
#include <fstream>
#include <chrono>
#include <ctime>
#include <mutex>

// levels of significance in increasing importance
enum class LogLevel {
    Info = 0,
    Warning = 1,
    Error = 2
};

// class Logger to realize recordings
class Logger {
private:
    std::ofstream loggerFile; // file for recordings
    LogLevel defaultLevel;
    std::mutex logMutex; // mutex for thread safety

public:
    Logger(const std::string& file, LogLevel level = LogLevel::Info);
    ~Logger();

    // method to change default level
    void setDefaultLevel(LogLevel level);

    // helpful method for recording to get string of current time
    std::string getCurrentTime();
    // from log level to string
    std::string levelToString(LogLevel level);
    //recording method
    void recording(const std::string& message, LogLevel level);

    // delete copy and assignment
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};
