#pragma once

#include <CLI/CLI.hpp>
#include <string>

/**
 * @brief Класс для отображения информации о фильтрах
 * 
 * Отвечает за:
 * - Вывод списка всех доступных фильтров
 * - Вывод информации о конкретном фильтре
 */
class FilterInfoDisplay
{
public:
    /**
     * @brief Выводит список всех доступных фильтров
     * @param app CLI::App для доступа к параметрам фильтров
     */
    static void printFilterList(CLI::App& app);

    /**
     * @brief Выводит информацию о конкретном фильтре
     * @param filter_name Имя фильтра
     * @param app CLI::App для доступа к параметрам фильтров
     */
    static void printFilterInfo(const std::string& filter_name, CLI::App& app);
};

