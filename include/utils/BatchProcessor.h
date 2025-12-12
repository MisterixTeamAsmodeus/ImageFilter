#pragma once

#include <string>
#include <vector>
#include <functional>
#include <filesystem>
#include <chrono>
#include <set>
#include <utils/FilterResult.h>

// Forward declaration
class IThreadPool;

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
 * @brief Информация о прогрессе обработки
 */
struct ProgressInfo
{
    size_t current;               // Номер текущего обрабатываемого файла (начиная с 1)
    size_t total;                 // Общее количество файлов
    std::string current_file;     // Путь к текущему обрабатываемому файлу
    double percentage;            // Процент выполнения (0.0 - 100.0)
    std::chrono::seconds elapsed_time;  // Прошедшее время в секундах
    std::chrono::seconds estimated_remaining;  // Оценка оставшегося времени в секундах
    double files_per_second;      // Скорость обработки (файлов в секунду)
};

/**
 * @brief Callback для отображения прогресса
 * @param info Информация о прогрессе обработки
 */
using ProgressCallback = std::function<void(const ProgressInfo& info)>;

/**
 * @brief Класс для пакетной обработки изображений
 * 
 * Поддерживает:
 * - Рекурсивный обход директорий
 * - Фильтрацию файлов по шаблону
 * - Сохранение структуры директорий
 * - Отображение прогресса с временем и ETA
 * - Обработку ошибок для отдельных файлов
 * - Возобновление прерванной обработки
 * - Параллельную обработку нескольких изображений
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
     *                         Принимает входной и выходной путь, возвращает FilterResult
     * @param progress_callback Callback для отображения прогресса (опционально)
     * @param thread_pool Пул потоков для параллельной обработки (nullptr = последовательная обработка)
     * @param max_parallel Максимальное количество параллельно обрабатываемых изображений (0 = автоматически)
     * @return Статистика обработки
     */
    BatchStatistics processAll(
        const std::function<FilterResult(const std::string&, const std::string&)>& process_function,
        ProgressCallback progress_callback = nullptr,
        IThreadPool* thread_pool = nullptr,
        int max_parallel = 0) const;

    /**
     * @brief Обрабатывает все найденные изображения с поддержкой возобновления
     * @param process_function Функция обработки одного изображения
     * @param progress_callback Callback для отображения прогресса (опционально)
     * @param resume_state_file Путь к файлу состояния для возобновления (пустая строка = без возобновления)
     * @param thread_pool Пул потоков для параллельной обработки (nullptr = последовательная обработка)
     * @param max_parallel Максимальное количество параллельно обрабатываемых изображений (0 = автоматически)
     * @return Статистика обработки
     */
    BatchStatistics processAllWithResume(
        const std::function<FilterResult(const std::string&, const std::string&)>& process_function,
        ProgressCallback progress_callback = nullptr,
        const std::string& resume_state_file = "",
        IThreadPool* thread_pool = nullptr,
        int max_parallel = 0) const;

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
};

