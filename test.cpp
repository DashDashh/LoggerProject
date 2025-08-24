#include "gtest/gtest.h"
#include "Logger.h"
#include <fstream>
#include <cstdio>

//test to check constructor
TEST(LoggerTest, Constructor) {
    const char* filename = "test_basic.log";

    // Удаляем файл если существует
    std::remove(filename);

    // Тестируем конструктор
    EXPECT_NO_THROW({
        Logger logger(filename, LogLevel::Info);
        });

    // Проверяем что файл создан
    std::ifstream file(filename);
    EXPECT_TRUE(file.good());
    file.close();

    // Убираем за собой
    std::remove(filename);
}

// open invalid file
TEST(LoggerTest, ConstructorWithInvalidFile) {
    EXPECT_THROW({
        Logger logger("/invalid/path/test.log", LogLevel::Info);
        }, std::runtime_error);
}

//test destructor
TEST(LoggerTest, DestructorClosesFile) {
    Logger* logger = new Logger("test_destructor.log", LogLevel::Info);
    delete logger;

    std::ifstream file("test_destructor.log");
    EXPECT_TRUE(file.good());
}

//test to check writing message
TEST(LoggerTest, WriteMessage) {
    const char* test_file = "write_test.log";
    std::remove(test_file);

    Logger logger(test_file, LogLevel::Info);
    logger.recording("Hello World", LogLevel::Info);

    // Проверяем что файл не пустой
    std::ifstream file(test_file);
    std::string content;
    std::getline(file, content);
    file.close();

    EXPECT_FALSE(content.empty());
    EXPECT_TRUE(content.find("Hello World") != std::string::npos);

    std::remove(test_file);
}

//test to check default level info

TEST(LoggerTest, InfoLevel) {
    const char* test_file = "write_test.log";
    std::remove(test_file);

    Logger logger(test_file, LogLevel::Info);
    logger.recording("This is Info", LogLevel::Info);
    logger.recording("This is Warning", LogLevel::Warning);
    logger.recording("This is Error", LogLevel::Error);

    // Проверяем что файл не пустой
    std::ifstream file(test_file);
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    file.close();

    EXPECT_FALSE(content.empty());
    EXPECT_TRUE(content.find("This is Info") != std::string::npos);
    EXPECT_TRUE(content.find("This is Warning") != std::string::npos);
    EXPECT_TRUE(content.find("This is Error") != std::string::npos);

    std::remove(test_file);
}

//test to check recording message with warning level
TEST(LoggerTest, WarningLevel) {
    const char* test_file = "level_test.log";
    std::remove(test_file);

    // Устанавливаем высокий уровень - только Error
    Logger logger(test_file, LogLevel::Warning);

    logger.recording("This is Info", LogLevel::Info);
    logger.recording("This is Warning", LogLevel::Warning);
    logger.recording("This is Error", LogLevel::Error);

    std::ifstream file(test_file);
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }

    file.close();

    EXPECT_TRUE(content.find("This is Info") == std::string::npos);
    EXPECT_TRUE(content.find("This is Warning") != std::string::npos);
    EXPECT_TRUE(content.find("This is Error") != std::string::npos);

    std::remove(test_file);
}

//test to check recording message with error level
TEST(LoggerTest, ErrorLevel) {
    const char* test_file = "level_test.log";
    std::remove(test_file);

    // Устанавливаем высокий уровень - только Error
    Logger logger(test_file, LogLevel::Error);

    logger.recording("This is Info", LogLevel::Info);
    logger.recording("This is Warning", LogLevel::Warning);
    logger.recording("This is Error", LogLevel::Error);

    std::ifstream file(test_file);
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }

    file.close();

    EXPECT_TRUE(content.find("This is Info") == std::string::npos);
    EXPECT_TRUE(content.find("This is Warning") == std::string::npos);
    EXPECT_TRUE(content.find("This is Error") != std::string::npos);

    std::remove(test_file);
}

//check data and time format
TEST(LoggerTest, LogFormatContainsTimestamp) {
    Logger logger("test_format.log", LogLevel::Info);
    logger.recording("Test message", LogLevel::Info);

    std::ifstream file("test_format.log");
    std::string content;
    std::getline(file, content);

    EXPECT_TRUE(content.find("20") != std::string::npos);
    EXPECT_TRUE(content.find("-") != std::string::npos);
    EXPECT_TRUE(content.find(":") != std::string::npos);
}

//check name of level in log
TEST(LoggerTest, LogFormatContainsLevel) {
    Logger logger("test_level_format.log", LogLevel::Info);
    logger.recording("Test message", LogLevel::Warning);

    std::ifstream file("test_level_format.log");
    std::string content;
    std::getline(file, content);

    EXPECT_TRUE(content.find("Warning:") != std::string::npos);
}

// test of thread safety
TEST(LoggerTest, ThreadSafetyMultipleWrites) {
    Logger logger("test_threads.log", LogLevel::Info);

    const int num_threads = 10;
    const int messages_per_thread = 100;

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&logger, i]() {
            for (int j = 0; j < messages_per_thread; ++j) {
                logger.recording("Thread " + std::to_string(i) +
                    " message " + std::to_string(j),
                    LogLevel::Info);
            }
            });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Проверяем что все сообщения записаны без падений
    std::ifstream file("test_threads.log");
    std::string content((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    // Должно быть num_threads * messages_per_thread строк
    size_t line_count = std::count(content.begin(), content.end(), '\n');
    EXPECT_EQ(line_count, num_threads * messages_per_thread);
}

//test to check function setDefaultLevel
TEST(LoggerTest, setDefaultLevel) {
    const char* test_file = "level_test.log";
    std::remove(test_file);

    Logger logger(test_file, LogLevel::Info);

    logger.recording("This is Info", LogLevel::Info);

    logger.setDefaultLevel(LogLevel::Warning);
    logger.recording("Wrong message", LogLevel::Info);
    logger.recording("This is Warning", LogLevel::Warning);

    std::ifstream file(test_file);
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    file.close();


    EXPECT_TRUE(content.find("This is Info") != std::string::npos);
    EXPECT_TRUE(content.find("Wrong message") == std::string::npos);
    EXPECT_TRUE(content.find("This is Warning") != std::string::npos);

    std::remove(test_file);
}
