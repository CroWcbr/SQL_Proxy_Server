#include "../include/Logger.hpp"
#include "../include/Proxy.hpp"
#include <string>
#include <signal.h>
#include <iostream>

bool Proxy::should_stop = false;

static void    _signal_handler(int signal)
{
    {
        std::string msg = "\rSTOP SIGNAL";
        if (signal == -1)
            msg.append(" Ctrl + D ");
        else if (signal == 2)
            msg.append(" Ctrl + C ");
        else if (signal == 3)
            msg.append(" Ctrl + / ");
        else if (signal == 20)
            msg.append(" Ctrl + Z ");

        msg.append("from user");

        std::cout << "\033[91m";
        std::cout << msg << std::endl;
        std::cout << "\033[0m";
    }
    Proxy::stop();
}

int main(int argc, char **argv)
{
    signal(SIGINT, _signal_handler);
    signal(SIGQUIT, _signal_handler);
    signal(SIGTSTP, _signal_handler);

    Proxy proxy(argc, argv);
    proxy.run();

    return 0;
}
