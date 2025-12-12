#pragma once

#include <filesystem>
#include <string>
#include <vector>

/**
 * @brief Вспомогательный класс для работы с файловой системой
 * 
 * Отвечает за:
 * - Поиск изображений в директориях
 * - Проверку типов файлов
 * - Сопоставление файлов с шаблонами
 * - Работу с путями
 */
class FileSystemHelper
{
public:
    /**
     * @brief Находит все изображения в директории
     * @param input_dir Входная директория
     * @param recursive Включить рекурсивный обход поддиректорий
     * @param pattern Шаблон для фильтрации файлов (например, "*.jpg")
     * @return Вектор путей к найденным изображениям
     */
    static std::vector<std::filesystem::path> findImages(
        const std::string& input_dir,
        bool recursive,
        const std::string& pattern);

    /**
     * @brief Проверяет, является ли файл изображением
     * @param path Путь к файлу
     * @return true если файл является изображением (JPEG или PNG)
     */
    static bool isImageFile(const std::filesystem::path& path);

    /**
     * @brief Проверяет, соответствует ли имя файла шаблону
     * @param filename Имя файла
     * @param pattern Шаблон (например, "*.jpg")
     * @return true если соответствует
     */
    static bool matchesPattern(const std::string& filename, const std::string& pattern);

    /**
     * @brief Получает относительный путь от базовой директории
     * @param full_path Полный путь к файлу
     * @param base_dir Базовая директория
     * @return Относительный путь
     */
    static std::filesystem::path getRelativePath(
        const std::filesystem::path& full_path,
        const std::filesystem::path& base_dir);

    /**
     * @brief Создает выходную директорию, если она не существует
     * @param output_path Путь к выходному файлу
     * @return true если директория создана или уже существует
     */
    static bool ensureOutputDirectory(const std::filesystem::path& output_path);

private:
    /**
     * @brief Рекурсивно находит изображения в директории
     * @param dir Директория для поиска
     * @param images Вектор для сохранения найденных путей
     * @param pattern Шаблон для фильтрации файлов
     */
    static void findImagesRecursive(
        const std::filesystem::path& dir,
        std::vector<std::filesystem::path>& images,
        const std::string& pattern);
};

