#pragma once

#include <cli/CommandHandler.h>
#include <string>

/**
 * @brief Класс для управления пресетами фильтров
 * 
 * Отвечает за:
 * - Загрузку пресетов из файлов
 * - Сохранение пресетов в файлы
 * - Преобразование между пресетами и цепочками фильтров
 */
class PresetManager
{
public:
    /**
     * @brief Загружает пресет и преобразует его в цепочку фильтров
     * @param preset_file Путь к файлу пресета
     * @param preset_dir Директория пресетов (для относительных путей)
     * @param filter_name Выходной параметр: цепочка фильтров
     * @return true если пресет успешно загружен, false в противном случае
     */
    static bool loadPreset(const std::string& preset_file,
                          const std::string& preset_dir,
                          std::string& filter_name);

    /**
     * @brief Сохраняет цепочку фильтров как пресет
     * @param filter_name Цепочка фильтров через запятую
     * @param preset_name Имя пресета
     * @param preset_dir Директория для сохранения пресета
     * @return true если пресет успешно сохранен, false в противном случае
     */
    static bool savePreset(const std::string& filter_name,
                          const std::string& preset_name,
                          const std::string& preset_dir);
};

