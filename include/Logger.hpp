#include <string>
#include <fstream>

#define LOG_DIR "./log"

enum LogType
{
    INFO,
    LOG,
    ERROR,
    DEBUG
};

class Logger
{
private:
    std::ofstream   file_stream;
    std::string     full_file_path;
    std::string     getCurrentDateTime();

public:
    Logger();
    Logger(const std::string& file_name);
    ~Logger();
	Logger(Logger const &copy) = delete;
	Logger &operator=(Logger const &copy) = delete;

    void log(const std::string& message);
    void log(const std::string& message, LogType type);
};
