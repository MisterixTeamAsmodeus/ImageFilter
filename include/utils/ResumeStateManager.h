#pragma once

#include <filesystem>
#include <string>
#include <set>

/**
 * @brief Класс для управления состоянием возобновления пакетной обработки
 * 
 * Отвечает за:
 * - Загрузку состояния возобновления из файла
 * - Сохранение состояния возобновления в файл
 * - Проверку, обработан ли файл
 */
class ResumeStateManager
{
public:
    /**
     * @brief Загружает состояние возобновления из файла
     * @param state_file Путь к файлу состояния
     * @return Множество путей к уже обработанным файлам
     */
    static std::set<std::string> loadResumeState(const std::string& state_file);

    /**
     * @brief Сохраняет состояние возобновления в файл
     * @param state_file Путь к файлу состояния
     * @param processed_files Множество путей к обработанным файлам
     * @return true если сохранение успешно
     */
    static bool saveResumeState(const std::string& state_file, const std::set<std::string>& processed_files);

    /**
     * @brief Проверяет, обработан ли файл (существует ли выходной файл)
     * @param output_path Путь к выходному файлу
     * @return true если файл уже обработан
     */
    static bool isFileProcessed(const std::filesystem::path& output_path);
};

