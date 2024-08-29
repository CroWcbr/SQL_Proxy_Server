#include "../include/Proxy.hpp"
#include <iostream>

#include <sstream>
#include <cstring>
#include <unistd.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

Proxy::Proxy(int argc, char **argv):
    log_query(LOG_QUERY)
    // , log_debug(LOG_DEBUG) // дефолтный конструктор по умолчанию в stdout
{
    if (!(_init_param(argc, argv) && _proxy_start()))
    {
        log_query.~Logger();
        log_debug.~Logger();
        exit(1);
    }

    fds.emplace_back(pollfd{proxy_fd, POLLIN, 0});
    connection[proxy_fd] = Connection{proxy_fd, proxy_fd, false, true};
    log_debug.log(LogType::INFO, "Proxy start on " + proxy_host + " port " + proxy_port + " on fd " + std::to_string(proxy_fd));
}

Proxy::~Proxy()
{
    for (auto it = fds.begin(); it != fds.end(); )
    {
        close(it->fd);
        connection.erase(it->fd);
        it = fds.erase(it);
    }    
}

bool Proxy::_init_param(int argc, char **argv)
{
    if (argc != 3)
    {
            log_debug.log(LogType::ERROR, "Error usage. Need ./* <proxy_port> <postgresql_host>:<postgresql_port>");
            return false;
    }
    proxy_host = PROXY_HOST;
    proxy_port = argv[1];


    std::vector<std::string> postgresql;
    std::stringstream ss(argv[2]);
    std::string item;
    while(std::getline(ss, item, ':'))
        postgresql.push_back(item);

    if (postgresql.size() != 2)
    {
            log_debug.log(LogType::ERROR, "Error input postgresql: <postgresql_host>:<postgresql_port>");
            return false;
    }

    postgresql_host = postgresql[0];
    postgresql_port = postgresql[1];    

    return true;
}

bool Proxy::_proxy_start()
{
    addrinfo hints{};
    addrinfo* res = nullptr;
    addrinfo* p = nullptr;
    
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(proxy_host.c_str(), proxy_port.c_str(), &hints, &res);
    if (status != 0)
    {
        log_debug.log(LogType::ERROR, "Error getting address info: " + std::string(gai_strerror(status)));
        return false;
    }

    for (p = res; p != nullptr; p = p->ai_next)
    {
        proxy_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (proxy_fd < 0)
            continue;

        int on = 1;
        if (setsockopt(proxy_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        {
            close(proxy_fd);
            continue;
        }

        if (bind(proxy_fd, p->ai_addr, p->ai_addrlen) == 0)
            break;

        close(proxy_fd);
    }

    freeaddrinfo(res);

    if (p == nullptr)
    {
        log_debug.log(LogType::ERROR, std::string("Error binding socket: ") + std::strerror(errno));
        return false;
    }

    if (fcntl(proxy_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        log_debug.log(LogType::ERROR, std::string("Error setting non-blocking mode: ") + std::strerror(errno));
        close(proxy_fd);
        return false;
    }

    if (listen(proxy_fd, MAX_LISTEN) < 0)
    {
        log_debug.log(LogType::ERROR, std::string("Error listening on socket: ") + std::strerror(errno));
        close(proxy_fd);
        return false;
    }

    return true;
}

void Proxy::run()
{
    // std::cout << "fds " << fds.size() << "\t : ";
    // for (auto f : fds)
    //     std::cout << f.fd << " ";
    // std::cout << std::endl;
    // std::cout << "connection " << connection.size() << "\t : ";
    // for (auto f : connection)
    //     std::cout << f.first << " ";
    // std::cout << std::endl;
    while(!should_stop)
    {
        int poll_count = poll(&(fds.front()), fds.size(), 1000);

        if (poll_count < 0)
        {
            log_debug.log(LogType::WARNING, std::string("poll error: ") + std::strerror(errno));
            continue;
        }

        if (poll_count == 0)
            continue;

        for (pollfdType::iterator it = fds.begin(), itend = fds.end(); it != itend; ++it)
        {
            if (it->revents == 0)
                continue;

            if (it->revents & POLLIN && it->fd == proxy_fd)
            {
                _poll_in_serv(it);
                break;
            }
            else if (!connection[it->fd].active)
                continue;
            else if (it->revents & POLLIN)
                _poll_in_connection(it);
            else if (it->revents & POLLOUT)
                _poll_out(it);
            else
                _poll_else(it);

            it->revents = 0;
        }

        for (auto it = fds.begin(); it != fds.end(); )
        {
            if (!connection[it->fd].active)
            {
                close(it->fd);
                connection.erase(it->fd);
                it = fds.erase(it);
            }
            else
                ++it;
        }

        // std::cout << "fds " << fds.size() << "\t : ";
        // for (auto f : fds)
        //     std::cout << f.fd << " ";
        // std::cout << std::endl;
        // std::cout << "connection " << connection.size() << "\t : ";
        // for (auto f : connection)
        //     std::cout << f.first << " ";
        // std::cout << std::endl;
    }
}

void Proxy::_poll_in_serv(pollfdType::iterator &it)
{
    // установка соединения
    log_debug.log(LogType::INFO, "_poll_in_serv : " + std::to_string(it->fd));

    int user_fd = accept(it->fd, nullptr, nullptr);
    if (user_fd < 0)
    {
        log_debug.log(LogType::ERROR, std::string("Accept error: ") + std::strerror(errno));
        return;
    }

    if (fcntl(user_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        log_debug.log(LogType::ERROR, std::string("Set nonblock error: ") + std::strerror(errno));
        close(user_fd);
        return;
    }

    addrinfo hints{};
    addrinfo* res = nullptr;

    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(postgresql_host.c_str(), postgresql_port.c_str(), &hints, &res);
    if (status != 0)
    {
        log_debug.log(LogType::ERROR, "Error getting address info: " + std::string(gai_strerror(status)));
        close(user_fd);
        return;
    }

    int remote_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (remote_fd < 0)
    {
        log_debug.log(LogType::ERROR, "Error create socket: " + std::string(gai_strerror(status)));
        close(user_fd);
        freeaddrinfo(res);
        return;
    }

    if (connect(remote_fd, res->ai_addr, res->ai_addrlen) < 0)
    {
        log_debug.log(LogType::ERROR, "Error create connect: " + std::string(gai_strerror(status)));
        close(user_fd);
        close(remote_fd);
        freeaddrinfo(res);
        return;
    }

    freeaddrinfo(res);

    if (fcntl(remote_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        log_debug.log(LogType::ERROR, "Error create nonblocking: " + std::string(gai_strerror(status)));
        close(user_fd);
        close(remote_fd);
        return;
    }

    fds.emplace_back(pollfd{user_fd, POLLIN, 0});
    fds.emplace_back(pollfd{remote_fd, POLLIN, 0});

    connection[user_fd] = Connection{user_fd, remote_fd, true,  true};
    connection[remote_fd] = Connection{remote_fd, user_fd, false, true};

    log_debug.log(LogType::INFO, "Client connect to proxy. Client fd " +  std::to_string(user_fd) + " Remote fd " + std::to_string(remote_fd));
}

void Proxy::_poll_in_connection(pollfdType::iterator &it)
{
    // пересылка сообщения
    // Драфт, я не ожидаю большие сообщения, работает в один заход, в будущем можно добавить цикл или отправку частями.
    log_debug.log(LogType::INFO, "_poll_in_connection : " +  std::to_string(it->fd) + " from " + (connection[it->fd].client ? "client" : "server"));

    char buffer[MAX_BUFFER_RECV];
    int nbytes = recv(it->fd, buffer, MAX_BUFFER_RECV - 1, 0);

    log_debug.log(LogType::DEBUG, "recv : " + std::to_string(nbytes) + " " + std::string(buffer, nbytes));
    if (nbytes < 0)
    {
        log_debug.log(LogType::ERROR, "Error read from : " +  std::to_string(it->fd));
        connection[it->fd].active = false;
        connection[connection[it->fd].to].active = false;
    }
    else if (nbytes == 0)
    {
        log_debug.log(LogType::INFO, "Ended session from : " +  std::to_string(it->fd));
        connection[it->fd].active = false;
        connection[connection[it->fd].to].active = false;
    }
    else
    {
        // Logging query
        if (buffer[0] == 'Q' && nbytes >= 5)
        {
            int32_t length = ntohl(*reinterpret_cast<int32_t*>(buffer + 1));
            if (length == nbytes - 1)
                log_query.log(buffer + 5, length);
        }

        int result = send(connection[it->fd].to, buffer, nbytes, 0);
        if (result < 0)
        {
            log_debug.log(LogType::ERROR, "Error send from : " +  std::to_string(it->fd) + " to " + std::to_string(connection[it->fd].to));
            connection[it->fd].active = false;
            connection[connection[it->fd].to].active = false;
            return ;
        }       
        it->events = POLLIN;
    }
}

void Proxy::_poll_out(pollfdType::iterator &it)
{
    // Я не сохраняю информацию на прокси, что получил, то и сразу отправил. Не должен сюда заходить.
    log_debug.log(LogType::WARNING, "_poll_out : " +  std::to_string(it->fd) + " from " + (connection[it->fd].client ? "client" : "server"));
}

void Proxy::_poll_else(pollfdType::iterator &it)
{
    // Проверка poll
    log_debug.log(LogType::WARNING, "_poll_else : " +  std::to_string(it->fd) + " from " + (connection[it->fd].client ? "client" : "server"));

    if (it->revents & POLLNVAL)
        log_debug.log(LogType::WARNING, "_PollElse SERVER_POLLNVAL :" +  std::to_string(it->fd) + " from " + (connection[it->fd].client ? "client" : "server"));
    else if (it->revents & POLLHUP)
        log_debug.log(LogType::WARNING, "_PollElse SERVER_POLLHUP :" +  std::to_string(it->fd) + " from " + (connection[it->fd].client ? "client" : "server"));
    else if (it->revents & POLLERR)
        log_debug.log(LogType::WARNING, "_PollElse SERVER_POLLERR :" +  std::to_string(it->fd) + " from " + (connection[it->fd].client ? "client" : "server"));
    
    connection[it->fd].active = false;
    connection[connection[it->fd].to].active = false;
}
