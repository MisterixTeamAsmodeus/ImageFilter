#pragma once

#include <string>
#include <vector>
#include <functional>
#include <filesystem>

/**
 * @brief Результат обработки одного файла в пакетном режиме
 */
struct BatchFileResult
{
    std::string input_path;      // Путь к входному файлу
    std::string output_path;      // Путь к выходному файлу
    bool success;                 // Успешна ли обработка
    std::string error_message;    // Сообщение об ошибке (если есть)
};

/**
 * @brief Статистика пакетной обработки
 */
struct BatchStatistics
{
    size_t total_files;           // Всего файлов найдено
    size_t processed_files;        // Успешно обработано
    size_t failed_files;          // Ошибок при обработке
    size_t skipped_files;         // Пропущено файлов
};

/**
 * @brief Callback для отображения прогресса
 * @param current Номер текущего обрабатываемого файла (начиная с 1)
 * @param total Общее количество файлов
 * @param current_file Путь к текущему обрабатываемому файлу
 */
using ProgressCallback = std::function<void(size_t current, size_t total, const std::string& current_file)>;

/**
 * @brief Класс для пакетной обработки изображений
 * 
 * Поддерживает:
 * - Рекурсивный обход директорий
 * - Фильтрацию файлов по шаблону
 * - Сохранение структуры директорий
 * - Отображение прогресса
 * - Обработку ошибок для отдельных файлов
 */
class BatchProcessor
{
public:
    /**
     * @brief Конструктор
     * @param input_dir Входная директория
     * @param output_dir Выходная директория
     * @param recursive Включить рекурсивный обход поддиректорий
     * @param pattern Шаблон для фильтрации файлов (например, "*.jpg", "*.png")
     */
    BatchProcessor(const std::string& input_dir, 
                   const std::string& output_dir,
                   bool recursive = false,
                   const std::string& pattern = "");

    /**
     * @brief Находит все изображения в входной директории
     * @return Вектор путей к найденным изображениям
     */
    std::vector<std::filesystem::path> findImages() const;

    /**
     * @brief Обрабатывает все найденные изображения
     * @param process_function Функция обработки одного изображения
     *                         Принимает входной и выходной путь, возвращает true при успехе
     * @param progress_callback Callback для отображения прогресса (опционально)
     * @return Статистика обработки
     */
    BatchStatistics processAll(
        const std::function<bool(const std::string&, const std::string&)>& process_function,
        ProgressCallback progress_callback = nullptr) const;

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

private:
    std::string input_dir_;
    std::string output_dir_;
    std::string pattern_;
    bool recursive_;

    /**
     * @brief Рекурсивно находит изображения в директории
     * @param dir Директория для поиска
     * @param images Вектор для сохранения найденных путей
     */
    void findImagesRecursive(const std::filesystem::path& dir, 
                            std::vector<std::filesystem::path>& images) const;

    /**
     * @brief Создает выходную директорию, если она не существует
     * @param output_path Путь к выходному файлу
     * @return true если директория создана или уже существует
     */
    bool ensureOutputDirectory(const std::filesystem::path& output_path) const;
};

