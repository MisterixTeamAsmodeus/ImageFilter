#pragma once

#include <cli/CommandHandler.h>
#include <utils/Logger.h>

/**
 * @brief Класс для настройки системы логирования на основе параметров командной строки
 * 
 * Отвечает за конфигурацию уровней логирования и режимов вывода.
 */
class LoggerConfigurator
{
public:
    /**
     * @brief Конструктор по умолчанию
     */
    LoggerConfigurator() = default;

    /**
     * @brief Деструктор
     */
    ~LoggerConfigurator() = default;

    // Запрещаем копирование
    LoggerConfigurator(const LoggerConfigurator&) = delete;
    LoggerConfigurator& operator=(const LoggerConfigurator&) = delete;

    /**
     * @brief Настраивает логирование на основе параметров командной строки
     * 
     * @param options Параметры команды, содержащие настройки логирования
     */
    static void configure(const CommandOptions& options);
};

