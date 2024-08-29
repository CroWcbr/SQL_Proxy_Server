#pragma once

# include <string>
# include <fcntl.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <cstring>
# include <poll.h>
# include <vector>
# include <iostream>

# define MAX_LISTEN 10
# define MAX_BUFFER_RECV 1024
# define HOST "127.0.0.1"
# define PORT 5555

class Proxy
{
private:
    typedef std::vector<struct pollfd>    pollfdType;
    pollfdType                            _fds;

    std::string                            _local_ip;
    int                                    _local_port;
    sockaddr_in                         _local_addr;
    int                                    _local_fd;

    std::string                            _request;

private:    
    Proxy();

    void    _PollInServ(pollfdType::iterator &it);
    void    _PollInUser(pollfdType::iterator &it);
    void    _PollElse(pollfdType::iterator &it);

public:
    Proxy(std::string addr_ip, int addr_port);
    ~Proxy();

    void Loop();
};