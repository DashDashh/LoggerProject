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

void Logger::recording(const std::string& message, LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);

    // check if we need to record this message
    if (static_cast<int>(level) >= static_cast<int>(defaultLevel)) {

        // define current time
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

        // record message
        loggerFile << "[" << timeBuffer << "] ";

        switch (level) {
        case LogLevel::Info:
            loggerFile << "Info: ";
            break;
        case LogLevel::Warning:
            loggerFile << "Warning: ";
            break;
        case LogLevel::Error:
            loggerFile << "Error: ";
            break;
        }

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
