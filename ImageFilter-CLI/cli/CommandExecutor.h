#pragma once

#include <cli/CommandHandler.h>

// Forward declaration
namespace CLI {
    class App;
}

/**
 * @brief Класс для выполнения команд
 * 
 * Отвечает за:
 * - Выполнение различных команд (обработка изображений, пакетная обработка, пресеты)
 * - Координацию работы компонентов
 * - Обработку ошибок
 */
class CommandExecutor
{
public:
    /**
     * @brief Конструктор исполнителя команд
     */
    CommandExecutor();

    /**
     * @brief Выполняет команду на основе опций
     * @param options Параметры команды
     * @param app CLI::App для доступа к параметрам фильтров
     * @return Код возврата (0 = успех, != 0 = ошибка)
     */
    int execute(const CommandOptions& options, CLI::App& app);

private:
    /**
     * @brief Выполняет команду вывода списка фильтров
     * @param app CLI::App для доступа к параметрам фильтров
     * @return Код возврата
     */
    int executeListFilters(CLI::App& app);

    /**
     * @brief Выполняет команду вывода информации о фильтре
     * @param filter_name Имя фильтра
     * @param app CLI::App для доступа к параметрам фильтров
     * @return Код возврата
     */
    int executeFilterInfo(const std::string& filter_name, CLI::App& app);

    /**
     * @brief Выполняет команду сохранения пресета
     * @param options Параметры команды
     * @return Код возврата
     */
    int executeSavePreset(const CommandOptions& options);

    /**
     * @brief Выполняет команду обработки одного изображения
     * @param options Параметры команды
     * @param app CLI::App для доступа к параметрам фильтров
     * @return Код возврата
     */
    int executeSingleImage(const CommandOptions& options, CLI::App& app);

    /**
     * @brief Выполняет команду пакетной обработки
     * @param options Параметры команды
     * @param app CLI::App для доступа к параметрам фильтров
     * @return Код возврата
     */
    int executeBatchProcessing(const CommandOptions& options, CLI::App& app);
};

