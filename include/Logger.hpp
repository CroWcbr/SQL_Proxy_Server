#pragma once

#include <string>
#include <fstream>

/// @brief Путь к директории для логов.
constexpr const char*  LOG_DIR = "./log";

/// @brief Флаг для включения режима отладки.
constexpr bool DEBUG_MODE = false;

/// @brief Перечисление типов логов.
enum LogType
{
    INFO,
    LOG,
    ERROR,
    DEBUG,
    WARNING,
};

/// @brief Класс для ведения логов в файл или в консоль.
class Logger
{
private:
    std::ofstream   file_stream;        ///< Поток для записи логов в файл.
    bool            debug{DEBUG_MODE};  ///< Флаг для включения режима отладки.

    /**
     * @brief Получает текущую дату и время в формате строки.
     * @return Строка с текущей датой и временем.
     */
    std::string     getCurrentDateTime();

public:
    /// @brief Конструктор, создающий логгер, выводящий логи в консоль.
    Logger();

    /**
     * @brief Конструктор, создающий логгер с выводом в файл.
     * @param file_name Имя файла для записи логов.
     */
    Logger(const std::string& file_name);

    /// @brief Деструктор. Закрывает файл, если он открыт.
    ~Logger();

    /// @brief Запрещает копирование логгера.
    Logger(Logger const &copy) = delete;

    /// @brief Запрещает присваивание логгеру копии.
    Logger &operator=(Logger const &copy) = delete;

    /// @brief Запрещает перемещение логгера.
    Logger(Logger&&) = delete;

    /// @brief Запрещает перемещение с присваиванием логгеру.
    Logger& operator=(Logger&&) = delete;

    /**
     * @brief Записывает сообщение в лог.
     * @param message Сообщение для записи.
     */
    void log(const std::string& message);

    /**
     * @brief Записывает сообщение в лог.
     * @param message Указатель на массив символов, представляющий сообщение.
     * @param length Длина сообщения в байтах.
     */
    void log(const char* message, size_t length);

    /**
     * @brief Записывает сообщение определенного типа в лог.
     * @param type Тип сообщения (INFO, LOG, ERROR, DEBUG, WARNING).
     * @param message Сообщение для записи.
     */
    void log(LogType type, const std::string& message);
};
