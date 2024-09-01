#pragma once

#include "../include/Logger.hpp"
#include "../include/Logger_query.hpp"
#include <string>
#include <vector>
#include <map>

#include <poll.h>

constexpr const char* LOG_QUERY = "log_query";
constexpr const char* LOG_DEBUG = "log_debug";
constexpr const char* PROXY_HOST = "127.0.0.1";
constexpr int MAX_LISTEN = 100;
constexpr int MAX_BUFFER_RECV = 65436; // если маленькое будут ошибки в логгировании, т.к. я не могу узнать длину первого сообщения и пропускаю его.

// Структура соединий
// to - сервер с которым связан
// client - клент или сервер
// active - для перевода в неактивное состояние и удаление после прохода всех соединений
// init - инициизирован ли (первое сообщение без длины, поэтому нужна доп переменная)
// len_query - длина запроса
// data - вектор символов запроса, если не влез в буфер[MAX_BUFFER_RECV]

struct Connection
{
    int                 to;
    bool                client;
    bool                active;
    bool                init;
    int                 len_query;
    std::vector<char>   data;
};

class Proxy
{
private:
// Logger
    Logger_query    log_query;  // логгер в отдельном потоке
    Logger          log_debug;  // обычный логгер

// вектор открытых соединений
    typedef std::vector<struct pollfd> pollfdType;
    pollfdType      fds;
    std::map<int, Connection> connection;

// Proxy server
    std::string     proxy_host;
    std::string     proxy_port;
    int             proxy_fd;

// PostgreSQL
    std::string     postgresql_host;
    std::string     postgresql_port;

// точка остановки
    static bool     should_stop;

private:
    bool    _init_param(int argc, char **argv);
    bool    _proxy_start();

    void    _poll_in_serv(pollfdType::iterator &it);
    void    _poll_in_connection(pollfdType::iterator &it);
    void    _poll_out(pollfdType::iterator &it);
    void    _poll_else(pollfdType::iterator &it);

public:
    Proxy() = delete;
    Proxy(int argc, char **argv);
    Proxy(Proxy const &copy) = delete;
    Proxy &operator=(Proxy const &copy) = delete;
    ~Proxy();

    static void stop() { should_stop = true; }

    void run();
};


