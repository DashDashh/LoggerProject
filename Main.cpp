#include "Logger.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

// Global objects for synchronization and queues
std::queue<std::pair<std::string, LogLevel>> messageQueue;
std::mutex queueMutex;
std::condition_variable queueCV;
std::atomic<bool> finished(false);
Logger* g_logger = nullptr; // pointer on Logger

// function for thread to record messages
void loggerThreadFunc() {
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCV.wait(lock, [] { return !messageQueue.empty() || finished.load(); });

        while (!messageQueue.empty()) {
            auto msg = messageQueue.front();
            messageQueue.pop();
            lock.unlock();
            g_logger->recording(msg.first, msg.second);
            lock.lock();
        }

        if (finished.load() && messageQueue.empty())
            break;
    }
}

// function for parseMessage to define level of message
LogLevel parseLevel(const std::string& levelStr) {
    std::string lvl = levelStr;
    std::transform(lvl.begin(), lvl.end(), lvl.begin(), ::tolower);
    if (lvl == "info")
        return LogLevel::Info;
    else if (lvl == "warning")
        return LogLevel::Warning;
    else if (lvl == "error")
        return LogLevel::Error;
    else
        return LogLevel::Info;
}

// function to parse message 
std::pair<std::string, LogLevel> parseMessage(const std::string& message) {
    size_t pos = message.find(':');
    if (pos != std::string::npos) {
        std::string prefix = message.substr(0, pos);
        std::string rest = message.substr(pos + 1);

        // Trim whitespace
        rest.erase(0, rest.find_first_not_of(" \t"));
        rest.erase(rest.find_last_not_of(" \t") + 1);

        LogLevel level = parseLevel(prefix);
        return std::make_pair(rest, level);
    }
    return std::make_pair(message, LogLevel::Info);
}

int main(int argc, char* argv[]) {

    // throw an error if we don't have even name of file
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <logfile> [default_level]" << std::endl;
        std::cerr << "Levels: Info, Warning, Error" << std::endl;
        return 1;
    }

    //take name of file
    std::string logFileName = argv[1];
    LogLevel defaultLevel = LogLevel::Info;

    // take default level if we have it in arguments
    if (argc >= 3) {
        std::string levelStr = argv[2];
        if (levelStr == "Info" || levelStr == "info")
            defaultLevel = LogLevel::Info;
        else if (levelStr == "Warning" || levelStr == "warning")
            defaultLevel = LogLevel::Warning;
        else if (levelStr == "Error" || levelStr == "error")
            defaultLevel = LogLevel::Error;
        else
            std::cerr << "Unknown log level, using Info by default." << std::endl;
    }

    // create logger and thread for records
    Logger logger(logFileName, defaultLevel);
    g_logger = &logger;

    std::thread loggerThread(loggerThreadFunc);

    // take message while we don't get string "exit"
    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "exit") {
            finished.store(true);
            queueCV.notify_all();
            break;
        }
        // if we have message "set level" we set new default level
        if (input.size() >= 10 && input.substr(0, 10) == "Set level:") {
            std::string levelStr = input.substr(11);
            LogLevel newLevel = parseLevel(levelStr);
            logger.setDefaultLevel(newLevel);
            std::cout << "Default log level changed to " << levelStr << std::endl;
            continue;
        }

        // parse message we get
        auto parsed = parseMessage(input);
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            messageQueue.push(parsed);
        }
        queueCV.notify_all();
    }

    loggerThread.join();

    return 0;
}
