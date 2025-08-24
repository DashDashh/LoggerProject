#include "Logger.h"
#include <stdexcept>
#include <iomanip>
#include <mutex>

Logger::Logger(const std::string& file, LogLevel level) : defaultLevel(level) {
    loggerFile.open(file, std::ios::app);
    if (!loggerFile) {
        throw std::runtime_error("Cannot open file: " + file);
    }
}

void Logger::setDefaultLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    defaultLevel = level;
}

std::string Logger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::tm timeinfo;
#ifdef _WIN32
    localtime_s(&timeinfo, &now_time);
#else
    localtime_r(&now_time, &timeinfo);
#endif

    char timeBuffer[20];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(timeBuffer);
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
    case LogLevel::Info:
        return "Info: ";
    case LogLevel::Warning:
        return "Warning: ";
    case LogLevel::Error:
        return "Error: ";
    }
}

void Logger::recording(const std::string& message, LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);

    // check if we need to record this message
    if (static_cast<int>(level) >= static_cast<int>(defaultLevel)) {

        // record time
        loggerFile << "[" << getCurrentTime() << "] ";

        //record level
        loggerFile << levelToString(level);

        //record message
        loggerFile << message << std::endl;
        loggerFile.flush();
    }
}


Logger::~Logger() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (loggerFile.is_open()) {
        loggerFile.close();
    }
}
