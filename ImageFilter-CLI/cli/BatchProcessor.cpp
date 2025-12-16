#include <cli/BatchProcessor.h>
#include <utils/FileSystemHelper.h>
#include <preset/ResumeStateManager.h>
#include <utils/FilterResult.h>
#include <utils/Logger.h>
#include <utils/IThreadPool.h>
#include <utils/ThreadPool.h>

#include <filesystem>
#include <set>
#include <mutex>
#include <atomic>
#include <chrono>

BatchProcessor::BatchProcessor(const std::string& input_dir,
                               const std::string& output_dir,
                               bool recursive,
                               const std::string& pattern)
    : input_dir_(input_dir)
    , output_dir_(output_dir)
    , pattern_(pattern)
    , recursive_(recursive)
{
}

std::vector<std::filesystem::path> BatchProcessor::findImages() const
{
    return FileSystemHelper::findImages(input_dir_, recursive_, pattern_);
}

BatchStatistics BatchProcessor::processAll(
    const std::function<FilterResult(const std::string&, const std::string&)>& process_function,
    ProgressCallback progress_callback,
    IThreadPool* thread_pool,
    int max_parallel) const
{
    return processAllWithResume(process_function, progress_callback, "", thread_pool, max_parallel);
}

BatchStatistics BatchProcessor::processAllWithResume(
    const std::function<FilterResult(const std::string&, const std::string&)>& process_function,
    ProgressCallback progress_callback,
    const std::string& resume_state_file,
    IThreadPool* thread_pool,
    int max_parallel) const
{
    BatchStatistics stats{};
    auto images = findImages();
    stats.total_files = images.size();

    if (images.empty())
    {
        Logger::warning("Не найдено изображений для обработки в директории: " + input_dir_);
        return stats;
    }

    Logger::info("Найдено изображений для обработки: " + std::to_string(stats.total_files));

    // Создаем выходную директорию, если она не существует
    std::filesystem::path output_path(output_dir_);
    try
    {
        std::filesystem::create_directories(output_path);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        Logger::error("Не удалось создать выходную директорию: " + std::string(e.what()));
        return stats;
    }

    // Загружаем состояние возобновления, если указан файл
    std::set<std::string> processed_files;
    if (!resume_state_file.empty())
    {
        processed_files = ResumeStateManager::loadResumeState(resume_state_file);
        if (!processed_files.empty())
        {
            Logger::info("Загружено состояние возобновления: " + std::to_string(processed_files.size()) + " файлов уже обработано");
        }
    }

    // Определяем количество параллельных потоков
    bool use_parallel = (thread_pool != nullptr);
    int num_parallel = 1;
    if (use_parallel)
    {
        if (max_parallel > 0)
        {
            num_parallel = max_parallel;
        }
        else
        {
            num_parallel = thread_pool->getThreadCount();
        }
    }

    // Время начала обработки
    const auto start_time = std::chrono::steady_clock::now();
    std::atomic<size_t> processed_count{0};  // Счетчик всех проверенных файлов

    // Мьютекс для синхронизации доступа к статистике и состоянию
    std::mutex stats_mutex;
    std::set<std::string> processed_in_session;

    // Функция обработки одного файла
    auto process_single_file = [&](size_t /*index*/, const std::filesystem::path& input_file) {
        std::string input_file_str = input_file.string();

        // Определяем выходной путь
        std::filesystem::path output_file;
        if (recursive_)
        {
            std::filesystem::path input_path(input_dir_);
            std::filesystem::path relative = FileSystemHelper::getRelativePath(input_file, input_path);
            output_file = output_path / relative;
        }
        else
        {
            output_file = output_path / input_file.filename();
        }

        std::string output_file_str = output_file.string();

        // Проверяем, не обработан ли уже файл
        bool should_skip = false;
        {
            std::lock_guard<std::mutex> lock(stats_mutex);
            if (processed_files.find(output_file_str) != processed_files.end() ||
                ResumeStateManager::isFileProcessed(output_file))
            {
                stats.skipped_files++;
                should_skip = true;
            }
        }

        if (should_skip)
        {
            // Обновляем счетчик и прогресс для пропущенных файлов
            size_t current = ++processed_count;
            if (progress_callback)
            {
                const auto current_time = std::chrono::steady_clock::now();
                const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    current_time - start_time);
                
                double percentage = (static_cast<double>(current) / static_cast<double>(stats.total_files)) * 100.0;
                
                double files_per_second = 0.0;
                std::chrono::seconds estimated_remaining(0);
                if (elapsed.count() > 0 && current > 0)
                {
                    files_per_second = static_cast<double>(current) / elapsed.count();
                    if (files_per_second > 0.0)
                    {
                        const size_t remaining = stats.total_files - current;
                        estimated_remaining = std::chrono::seconds(
                            static_cast<long long>(remaining / files_per_second));
                    }
                }

                ProgressInfo info;
                info.current = current;
                info.total = stats.total_files;
                info.current_file = input_file_str;
                info.percentage = percentage;
                info.elapsed_time = elapsed;
                info.estimated_remaining = estimated_remaining;
                info.files_per_second = files_per_second;

                progress_callback(info);
            }
            return;
        }

        // Создаем выходную директорию для файла, если нужно
        if (!FileSystemHelper::ensureOutputDirectory(output_file))
        {
            std::lock_guard<std::mutex> lock(stats_mutex);
            Logger::error("Не удалось создать директорию для: " + output_file_str);
            stats.failed_files++;
            return;
        }

        // Обрабатываем файл
        FilterResult result;
        try
        {
            result = process_function(input_file_str, output_file_str);
        }
        catch (const std::exception& e)
        {
            std::lock_guard<std::mutex> lock(stats_mutex);
            stats.failed_files++;
            Logger::error("Ошибка при обработке " + input_file_str + ": " + e.what());
            return;
        }

        // Обновляем статистику
        size_t current = 0;
        {
            std::lock_guard<std::mutex> lock(stats_mutex);
            if (result.isSuccess())
            {
                stats.processed_files++;
                processed_in_session.insert(output_file_str);
                Logger::debug("Обработан: " + input_file_str + " -> " + output_file_str);
            }
            else
            {
                stats.failed_files++;
                Logger::warning("Не удалось обработать: " + input_file_str + 
                              ". Ошибка: " + result.getFullMessage());
            }

            current = ++processed_count;

            // Периодически сохраняем состояние возобновления
            if (!resume_state_file.empty() && current % 10 == 0)
            {
                std::set<std::string> all_processed = processed_files;
                all_processed.insert(processed_in_session.begin(), processed_in_session.end());
                ResumeStateManager::saveResumeState(resume_state_file, all_processed);
            }
        }

        // Обновляем прогресс
        if (progress_callback)
        {
            const auto current_time = std::chrono::steady_clock::now();
            const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                current_time - start_time);
            
            double percentage = (static_cast<double>(current) / static_cast<double>(stats.total_files)) * 100.0;
            
            double files_per_second = 0.0;
            std::chrono::seconds estimated_remaining(0);
            if (elapsed.count() > 0 && current > 0)
            {
                files_per_second = static_cast<double>(current) / elapsed.count();
                if (files_per_second > 0.0)
                {
                    const size_t remaining = stats.total_files - current;
                    estimated_remaining = std::chrono::seconds(
                        static_cast<long long>(remaining / files_per_second));
                }
            }

            ProgressInfo info;
            info.current = current;
            info.total = stats.total_files;
            info.current_file = input_file_str;
            info.percentage = percentage;
            info.elapsed_time = elapsed;
            info.estimated_remaining = estimated_remaining;
            info.files_per_second = files_per_second;

            progress_callback(info);
        }
    };

    // Параллельная или последовательная обработка
    if (use_parallel && num_parallel > 1)
    {
        // Параллельная обработка
        size_t current_index = 0;
        std::mutex index_mutex;

        // Создаем задачи для пула потоков
        for (int i = 0; i < num_parallel && i < static_cast<int>(images.size()); ++i)
        {
            thread_pool->enqueue([&]() {
                while (true)
                {
                    size_t idx;
                    {
                        std::lock_guard<std::mutex> lock(index_mutex);
                        if (current_index >= images.size())
                        {
                            break;
                        }
                        idx = current_index++;
                    }
                    process_single_file(idx, images[idx]);
                }
            });
        }

        // Ждем завершения всех задач
        thread_pool->waitAll();
    }
    else
    {
        // Последовательная обработка
        for (size_t i = 0; i < images.size(); ++i)
        {
            process_single_file(i, images[i]);
        }
    }

    // Финальное сохранение состояния возобновления
    if (!resume_state_file.empty())
    {
        std::set<std::string> all_processed = processed_files;
        all_processed.insert(processed_in_session.begin(), processed_in_session.end());
        ResumeStateManager::saveResumeState(resume_state_file, all_processed);
    }

    return stats;
}

std::filesystem::path BatchProcessor::getRelativePath(
    const std::filesystem::path& full_path,
    const std::filesystem::path& base_dir)
{
    // Используем FileSystemHelper для получения относительного пути
    const auto relative = FileSystemHelper::getRelativePath(full_path, base_dir);

    // На разных платформах строковое представление пути может отличаться.
    // Для кроссплатформенных тестов используем generic_string(), чтобы
    // получить путь с разделителями '/' независимо от ОС.
    //
    // Возвращаем новый std::filesystem::path, построенный из generic_string(),
    // чтобы вызов .string() в тестах возвращал единообразный вид
    // (например, "sub/file.png" вместо "sub\\file.png" на Windows).
    return std::filesystem::path(relative.generic_string());
}

bool BatchProcessor::isImageFile(const std::filesystem::path& path)
{
    return FileSystemHelper::isImageFile(path);
}

bool BatchProcessor::matchesPattern(const std::string& filename, const std::string& pattern)
{
    return FileSystemHelper::matchesPattern(filename, pattern);
}

