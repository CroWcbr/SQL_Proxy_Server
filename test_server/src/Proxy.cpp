#include "../include/Proxy.hpp"

Proxy::Proxy(std::string addr_ip, int addr_port)
{
    try
    {
        _local_ip = addr_ip;
        _local_port = addr_port;

        memset(&_local_addr, 0, sizeof(_local_addr));
        _local_addr.sin_family = AF_INET;
        _local_addr.sin_addr.s_addr = inet_addr(_local_ip.c_str());
        _local_addr.sin_port = htons(_local_port);

        if ((_local_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            throw std::runtime_error("Error socket");
        
        int on = 1;
        if (setsockopt(_local_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
            throw std::runtime_error("Error setsockopt");
        

        if (fcntl(_local_fd, F_SETFL, O_NONBLOCK) < 0)
            throw std::runtime_error("Error fcntl");    

        if (bind(_local_fd, reinterpret_cast<struct sockaddr*>(&_local_addr), sizeof(_local_addr)) < 0)
            throw std::runtime_error("Error bind");

        if (listen(_local_fd, MAX_LISTEN) < 0)
            throw std::runtime_error("Error listen");

        struct pollfd tmp;
        tmp.fd = _local_fd;
        tmp.events = POLLIN;
        tmp.revents = 0;
        _fds.push_back(tmp);
    }
    catch (std::exception& e)
    {
        std::cout << "Server start ERROR" << std::endl;
        _exit(1);
    }
    std::cout << "Server start on " << HOST<< ":" << PORT << std::endl;
}

Proxy::~Proxy()
{}

void Proxy::Loop()
{
    while (true)
    {
        int poll_count = poll(&(_fds.front()), _fds.size(), 1000);
        if (poll_count < 0)
        {
            std::cout << "poll_count < 0" << std::endl;
            continue;
        }

        if (poll_count == 0)
            continue;

        for (pollfdType::iterator it = _fds.begin(), itend = _fds.end(); it != itend; ++it)
        {
            if (it->revents == 0)
                continue;

            if (it->revents & POLLIN && it->fd == _local_fd)
                _PollInServ(it);
            else if (it->revents & POLLIN)
                _PollInUser(it);
            else
                _PollElse(it);
            it->revents = 0;
            break;
        }
    }    
}

void Proxy::_PollInServ(pollfdType::iterator &it)
{
    std::cout << "Poll in serv: " << it->fd << std::endl;

    sockaddr_in    addr;
    socklen_t addr_len = sizeof(addr);
    int user_fd = accept(it->fd, reinterpret_cast<struct sockaddr*>(&addr), &addr_len);
    if (user_fd < 0)
        std::cout << "ERROR_SERVER_POOLIN_NEW_CONN" << std::endl;
    else
    {
        char buffer[16];
        inet_ntop( AF_INET, &addr.sin_addr, buffer, sizeof(buffer));
        std::cout << "SERVER_POOLIN_NEW_CONN " << user_fd << " from : " << buffer << std::endl;

        struct pollfd tmp;
        tmp.fd = user_fd;
        tmp.events = POLLIN;
        tmp.revents = 0;

        if (fcntl(user_fd, F_SETFL, O_NONBLOCK) < 0)
        {
            std::cout << "ERROR_SERVER_POOLIN_FCNTL" << std::endl;
            close(user_fd);
            return;
        }
        _fds.push_back(tmp);
    }
}

void Proxy::_PollInUser(pollfdType::iterator &it)
{
    std::cout << "SERVER_POOLIN_RECIEVED_USER : " << it->fd << std::endl;

    char buffer[MAX_BUFFER_RECV];
    int nbytes = recv(it->fd, buffer, MAX_BUFFER_RECV - 1, 0);
    
    if (nbytes < 0)
    {
        std::cout << "ERROR_SERVER_POOLIN_USER_READ : " << it->fd << std::endl;
        close(it->fd);
        _fds.erase(it);
    }
    else if (nbytes == 0)
    {
        std::cout << "SERVER_POOLIN_USER_SESS_END : " << it->fd << std::endl;
        close(it->fd);
        _fds.erase(it);
    }
    else
    {    
        std::cout << "SERVER_POOLIN_USER_READ_END : " << it->fd << std::endl;
        if (buffer[0] == 'Q')
            _request = "";
        else
        {
            _request = "Answer : ";
            _request.append(buffer, nbytes);
        }

        int result;
        while ((result = send(it->fd, _request.c_str(), _request.length(), 0)) > 0)
            _request = _request.substr(result, _request.size());

        if (result < 0)
        {
            std::cout << "ERROR_SERVER_POOLIN_USER_SEND : " << it->fd << std::endl;
            close(it->fd);
            _fds.erase(it);
            return;
        }
        std::cout << "SERVER_POOLOUT_USER_SEND_END : " << it->fd << std::endl;
    }
}

void Proxy::_PollElse(pollfdType::iterator &it)
{    
    if ((*it).revents & POLLNVAL)
        std::cout << "SERVER_POLLNVAL : " << it->fd << " : ";
    else if ((*it).revents & POLLHUP)
        std::cout << "SERVER_POLLHUP : " << it->fd << " : ";
    else if ((*it).revents & POLLERR)
        std::cout << "SERVER_POLLERR : " << it->fd << " : ";

    close(it->fd);
    _fds.erase(it);
}
