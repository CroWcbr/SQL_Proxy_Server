#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <fstream>
#include <string>

/// @brief Путь к директории для логов.
constexpr const char*  LOG_Q_DIR = "./log";

class Logger_query
{
private:
    std::ofstream                   file_stream;    ///< Поток для записи логов в файл.
    std::queue<std::vector<char>>   log_queue;      ///< Очередь для хранения сообщений лога.
    std::mutex                      queue_mutex;    ///< Мьютекс для синхронизации доступа к очереди.
    std::condition_variable         cv;             ///< Условная переменная для уведомления потока записи.
    std::thread                     log_thread;     ///< Поток для обработки и записи сообщений лога. 
    std::atomic<bool>               done{false};    ///< Флаг для завершения работы потока.

    /**
     * @brief Рабочий метод для потока логгирования.
     * 
     * Этот метод выполняется в отдельном потоке и занимается обработкой очереди сообщений,
     * записывая их в file_stream.
     */
    void            log_worker();

    /**
     * @brief Получает текущую дату и время в формате строки.
     * @return Строка с текущей датой и временем.
     */
    std::string     getCurrentDateTime();

public:
    /// @brief Конструктор, создающий логгер, выводящий логи в консоль.
    Logger_query();

    /**
     * @brief Конструктор, создающий логгер с выводом в файл.
     * @param file_name Имя файла для записи логов.
     */
    Logger_query(const std::string& file_name);

    /// @brief Деструктор. Завершает работу потока и закрывает файл, если он открыт.
    ~Logger_query();

    /// @brief Запрещает копирование логгера.
    Logger_query(Logger_query const &copy) = delete;

    /// @brief Запрещает присваивание логгеру копии.
    Logger_query &operator=(Logger_query const &copy) = delete;

    /// @brief Запрещает перемещение логгера.
    Logger_query(Logger_query&&) = delete;

    /// @brief Запрещает перемещение с присваиванием логгеру.
    Logger_query& operator=(Logger_query&&) = delete;

    /**
     * @brief Добавляет сообщение в очередь на запись в лог.
     * @param message Сообщение для записи (передается через перемещение).
     */
    void log(std::vector<char>&& message);
};
