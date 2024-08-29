#include "../include/Logger.hpp"
#include <filesystem>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <chrono>
#include <cstdlib>

Logger::Logger()
{
    file_stream.copyfmt(std::cout);
    file_stream.basic_ios::rdbuf(std::cout.rdbuf());

    if (!file_stream) 
    {
        std::cerr << "Error redirecting stream to terminal" << std::endl;
        exit(EXIT_FAILURE);
    }
}

Logger::Logger(const std::string& file_name)
{
    if (!std::filesystem::exists(LOG_DIR)) 
    {
        try
        {
            std::filesystem::create_directories(LOG_DIR);
        }
        catch (std::filesystem::filesystem_error& e)
        {
            std::cerr << "Error creating directory: " << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    else if (!std::filesystem::is_directory(LOG_DIR))
    {
        std::cerr << "Path exists but is not a directory" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    full_file_path = std::string(LOG_DIR) + "/" + file_name;

    file_stream.open(full_file_path, std::ios::app);

    if (!file_stream.is_open())
    {
        std::cerr << "Error opening file " << full_file_path << " : " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Logger to " << full_file_path << " start" << std::endl;
}

Logger::~Logger()
{
    if (file_stream.is_open())
        file_stream.close();
}

void Logger::log(const std::string& message)
{
    file_stream << getCurrentDateTime() << message << std::endl;
}

void Logger::log(const char* message, size_t length)
{
    file_stream << getCurrentDateTime();
    file_stream.write(message, length);
    file_stream << std::endl;
}

void Logger::log(LogType type, const std::string& message)
{
    switch (type)
    {
        case LogType::INFO:
            file_stream << "[INFO] ";
            break;
        case LogType::LOG:
            file_stream << "[LOG] ";
            break;
        case LogType::ERROR:
            file_stream << "[ERROR] ";
            break;
        case LogType::DEBUG:
            file_stream << "[DEBUG] ";
            break;
        case LogType::WARNING:
            file_stream << "[WARNING] ";
            break;
        default:
            file_stream << "[UNKNOWN] ";
            break;
    }
    file_stream << getCurrentDateTime() << message << std::endl;
}

std::string Logger::getCurrentDateTime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "[%d/%m/%Y-%H:%M:%S]");
    ss << ": ";
    return ss.str();
}
