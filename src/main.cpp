#include "../include/Logger.hpp"
#include <string>
#include <signal.h>
#include <iostream>

// for test
#include <unistd.h>

static void	_signal_handler(int signal)
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
	exit(signal);
}

int main()
{
    signal(SIGINT, _signal_handler);	//Ctrl + 'C'
	signal(SIGQUIT, _signal_handler);	//Ctrl + '\'
	signal(SIGTSTP, _signal_handler);	//Ctrl + 'Z'

	Logger log_query("log_query");
	Logger log_debug;

    while (true)
	{
		sleep(1);
		log_query.log("123");
		log_debug.log("123", LogType::INFO);
    }

    return 0;
}
