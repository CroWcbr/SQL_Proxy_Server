#pragma once

#include "../include/Logger.hpp"
#include <string>
#include <vector>
#include <map>

#include <poll.h>

#define LOG_QUERY "log_query"
#define LOG_DEBUG "log_debug"
#define PROXY_HOST "127.0.0.1"
#define MAX_LISTEN 100
# define MAX_BUFFER_RECV 65536

struct Connection
{
    int     from;
    int     to;
    bool    client;
    bool    active;
};

class Proxy
{
private:
// Logger
    Logger          log_query;
    Logger          log_debug;

    typedef std::vector<struct pollfd> pollfdType;
    pollfdType      fds;

// Proxy server
    std::string     proxy_host;
    std::string     proxy_port;
    int             proxy_fd;

// PostgreSQL
    std::string     postgresql_host;
    std::string     postgresql_port;

    std::map<int, Connection> connection;

    static bool     should_stop;

private:
    bool            _init_param(int argc, char **argv);
    bool            _proxy_start();

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


