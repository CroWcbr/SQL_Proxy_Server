#include "../include/Logger_query.hpp"
#include <iostream>
#include <filesystem>
#include <cstring>

Logger_query::Logger_query()
{
    file_stream.copyfmt(std::cout);
    file_stream.basic_ios::rdbuf(std::cout.rdbuf());

    if (!file_stream) 
    {
        std::cerr << "Error redirecting stream to terminal" << std::endl;
        exit(EXIT_FAILURE);
    }
    log_thread = std::thread(&Logger_query::log_worker, this);
    std::cout << "Logger to terminal start" << std::endl;
}

Logger_query::Logger_query(const std::string& file_name)
{
    if (!std::filesystem::exists(LOG_Q_DIR)) 
    {
        try
        {
            std::filesystem::create_directories(LOG_Q_DIR);
        }
        catch (std::filesystem::filesystem_error& e)
        {
            std::cerr << "Error creating directory: " << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    else if (!std::filesystem::is_directory(LOG_Q_DIR))
    {
        std::cerr << "Path exists but is not a directory" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    std::string full_file_path = std::string(LOG_Q_DIR) + "/" + file_name;

    file_stream.open(full_file_path, std::ios::app);

    if (!file_stream.is_open())
    {
        std::cerr << "Error opening file " << full_file_path << " : " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    log_thread = std::thread(&Logger_query::log_worker, this);
    std::cout << "Logger to " << full_file_path << " start" << std::endl;
}

Logger_query::~Logger_query()
{
    done = true;
    cv.notify_all();
    if (log_thread.joinable())
        log_thread.join();

    if (file_stream.is_open())
        file_stream.close();
}

void Logger_query::log(std::vector<char>&& message)
{
    std::lock_guard<std::mutex> lock(queue_mutex);
    log_queue.push(std::move(message));
    cv.notify_one();
}

void Logger_query::log_worker()
{
    while (!done || !log_queue.empty())
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        cv.wait(lock, [&] { return done || !log_queue.empty(); });

        while (!log_queue.empty())
        {
            auto message = std::move(log_queue.front());
            log_queue.pop();
            lock.unlock();

            file_stream << getCurrentDateTime();

            file_stream.write(message.data(), message.size());
            // для тестов sysbench, что бы не разрастался бесконечно лог файл ))))
            // if (message.size() > 1000)
            // {
            //     file_stream << std::to_string(message.size()) << "\t";
            //     file_stream.write(message.data(), 1000);
            // } 
            // else
            //     file_stream.write(message.data(), message.size());

            file_stream << std::endl;
                

            lock.lock();
        }
    }
}

std::string Logger_query::getCurrentDateTime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "[%d/%m/%Y-%H:%M:%S]");
    ss << ": ";
    return ss.str();
}
