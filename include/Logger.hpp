#pragma once

#include <string>
#include <fstream>

constexpr const char*  LOG_DIR = "./log";
constexpr bool DEBUG_MODE = false;

enum LogType
{
    INFO,
    LOG,
    ERROR,
    DEBUG,
    WARNING,
};

class Logger
{
private:
    std::ofstream   file_stream;
    bool            debug{DEBUG_MODE};

    std::string     getCurrentDateTime();

public:
    Logger();
    Logger(const std::string& file_name);
    ~Logger();
    Logger(Logger const &copy) = delete;
    Logger &operator=(Logger const &copy) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    void log(const std::string& message);
    void log(const char* message, size_t length);
    void log(LogType type, const std::string& message);
};
