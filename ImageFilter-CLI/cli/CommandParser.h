#pragma once

#include <cli/CommandHandler.h>
#include <CLI/CLI.hpp>
#include <string>

/**
 * @brief Класс для парсинга аргументов командной строки
 * 
 * Отвечает за:
 * - Настройку CLI::App с опциями
 * - Парсинг аргументов командной строки
 * - Заполнение структуры CommandOptions
 */
class CommandParser
{
public:
    /**
     * @brief Конструктор парсера команд
     */
    CommandParser();

    /**
     * @brief Парсит аргументы командной строки
     * @param argc Количество аргументов
     * @param argv Массив аргументов
     * @param options Структура для заполнения параметрами
     * @return Код возврата (0 = успех, != 0 = ошибка парсинга)
     */
    int parse(int argc, char* argv[], CommandOptions& options);

    /**
     * @brief Получает CLI::App для доступа к параметрам фильтров
     * @return Ссылка на CLI::App
     */
    CLI::App& getApp() noexcept;

private:
    /**
     * @brief Настраивает опции командной строки
     */
    void setupOptions(CommandOptions& options);

    CLI::App app_;  // CLI11 приложение для парсинга
};

