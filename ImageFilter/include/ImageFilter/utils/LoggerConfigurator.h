#pragma once

#include <string>

/**
 * @brief Класс для настройки системы логирования
 *
 * Отвечает за конфигурацию уровней логирования и режимов вывода на основе
 * уже разобранных параметров (quiet и строкового уровня логирования).
 */
class LoggerConfigurator
{
public:
    /// Конструктор по умолчанию
    LoggerConfigurator() = default;

    /// Деструктор
    ~LoggerConfigurator() = default;

    // Запрещаем копирование
    LoggerConfigurator(const LoggerConfigurator&) = delete;
    LoggerConfigurator& operator=(const LoggerConfigurator&) = delete;

    /**
     * @brief Настраивает логирование
     *
     * @param quiet      Флаг тихого режима (отключает вывод)
     * @param log_level  Строковое представление уровня логирования
     *                   (например, "DEBUG", "INFO", "WARNING", "ERROR").
     */
    static void configure(bool quiet, const std::string& log_level);
};

