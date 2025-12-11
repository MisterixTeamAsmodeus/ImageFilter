#pragma once

#include <string>
#include <iostream>
#include <sstream>

/**
 * @brief Уровни логирования
 */
enum class LogLevel
{
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

/**
 * @brief Простой логгер для приложения
 * 
 * Предоставляет структурированное логирование с уровнями.
 * Можно отключить вывод для автоматизации через setQuiet().
 */
class Logger
{
public:
    /**
     * @brief Устанавливает минимальный уровень логирования
     * @param level Минимальный уровень (сообщения ниже этого уровня не выводятся)
     */
    static void setLevel(LogLevel level) noexcept;

    /**
     * @brief Устанавливает тихий режим (отключает весь вывод)
     * @param quiet true для отключения вывода
     */
    static void setQuiet(bool quiet) noexcept;

    /**
     * @brief Получает текущий минимальный уровень логирования
     * @return Текущий уровень
     */
    static LogLevel getLevel() noexcept;

    /**
     * @brief Проверяет, включен ли тихий режим
     * @return true если тихий режим включен
     */
    static bool isQuiet() noexcept;

    /**
     * @brief Логирует сообщение с указанным уровнем
     * @param level Уровень логирования
     * @param message Сообщение для вывода
     */
    static void log(LogLevel level, const std::string& message);

    /**
     * @brief Логирует DEBUG сообщение
     */
    static void debug(const std::string& message);

    /**
     * @brief Логирует INFO сообщение
     */
    static void info(const std::string& message);

    /**
     * @brief Логирует WARNING сообщение
     */
    static void warning(const std::string& message);

    /**
     * @brief Логирует ERROR сообщение
     */
    static void error(const std::string& message);
};
