#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <fstream>
#include <string>

constexpr const char*  LOG_Q_DIR = "./log";

class Logger_query
{
private:
    void log_worker();
    std::string getCurrentDateTime();

    std::ofstream                   file_stream;
    std::queue<std::vector<char>>   log_queue;
    std::mutex                      queue_mutex;
    std::condition_variable         cv;
    std::thread                     log_thread;
    std::atomic<bool>               done{false};

public:
    Logger_query() = delete;
    Logger_query(const std::string& file_name);
    ~Logger_query();
    Logger_query(Logger_query const &copy) = delete;
    Logger_query &operator=(Logger_query const &copy) = delete;

    void log(std::vector<char>&& message); // Используем rvalue reference для передачи полной памяти
};