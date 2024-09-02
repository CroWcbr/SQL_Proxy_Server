#include "../include/Logger.hpp"
#include "../include/Proxy.hpp"
#include <string>
#include <signal.h>
#include <iostream>

/// @brief Статическая переменная для остановки прокси-сервера.
bool Proxy::should_stop = false;

/**
 * @brief Обработчик сигналов.
 * 
 * Функция обрабатывает различные сигналы, такие как SIGINT, SIGQUIT, SIGTSTP и SIGPIPE.
 * 
 * @param signal Сигнал, который нужно обработать.
 */
static void    _signal_handler(int signal)
{
    std::string msg = "\r SIGNAL";
    if (signal == SIGPIPE)
    {
        msg.append(" SIGPIPE (Broken pipe) ");
        std::cout << msg << std::endl;
    }
    else
    {
        if (signal == SIGINT)
            msg.append(" SIGINT (Interrupt from keyboard) ");
        else if (signal == SIGQUIT)
            msg.append(" SIGQUIT (Quit from keyboard) ");
        else if (signal == SIGTSTP)
            msg.append(" SIGTSTP (Terminal stop) ");

        msg.append("from user");

        std::cout << "\033[91m";
        std::cout << msg << std::endl;
        std::cout << "\033[0m";
        
        Proxy::stop();
    }
}

/**
 * @brief Точка входа в программу.
 * 
 * Функция `main` инициализирует сигнал-обработчики, создает объект `Proxy` и запускает прокси-сервер.
 * 
 * Программа ожидает, что при запуске ей будут переданы следующие аргументы командной строки:
 * - `argc[1]`: Порт, на котором будет работать прокси-сервер.
 * - `argc[2]`: IP-адрес и порт, к которому будет подключаться прокси-сервер <ip>:<port>.
 * - Другие параметры, которые могут быть необходимы для конфигурации прокси-сервера.
 * 
 * @param argc Количество аргументов командной строки.
 * @param argv Массив аргументов командной строки.
 * @return Код завершения программы (0 при успешном завершении).
 */
int main(int argc, char **argv)
{
    signal(SIGINT, _signal_handler);
    signal(SIGQUIT, _signal_handler);
    signal(SIGTSTP, _signal_handler);
    signal(SIGPIPE, _signal_handler);

    Proxy proxy(argc, argv);
    proxy.run();

    return 0;
}
