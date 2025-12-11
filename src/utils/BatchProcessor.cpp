#include <utils/BatchProcessor.h>
#include <utils/Logger.h>

#include <filesystem>
#include <algorithm>
#include <cctype>

namespace
{
    /**
     * @brief Преобразует строку в нижний регистр
     */
    std::string toLower(const std::string& str)
    {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    /**
     * @brief Проверяет расширение файла
     */
    bool hasExtension(const std::filesystem::path& path, const std::vector<std::string>& extensions)
    {
        if (!path.has_extension())
        {
            return false;
        }

        std::string ext = toLower(path.extension().string());
        // Убираем точку из расширения
        if (!ext.empty() && ext[0] == '.')
        {
            ext = ext.substr(1);
        }

        for (const auto& valid_ext : extensions)
        {
            if (ext == valid_ext)
            {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Простой паттерн-матчинг для шаблонов вида "*.jpg"
     */
    bool simplePatternMatch(const std::string& filename, const std::string& pattern)
    {
        if (pattern.empty())
        {
            return true;
        }

        // Простая поддержка шаблонов вида "*.ext" или "*.EXT"
        if (pattern.size() >= 2 && pattern[0] == '*' && pattern[1] == '.')
        {
            std::string pattern_ext = pattern.substr(2);
            std::string filename_lower = toLower(filename);
            std::string pattern_ext_lower = toLower(pattern_ext);

            // Проверяем, заканчивается ли имя файла на расширение из шаблона
            if (filename_lower.size() >= pattern_ext_lower.size())
            {
                std::string file_ext = filename_lower.substr(
                    filename_lower.size() - pattern_ext_lower.size());
                return file_ext == pattern_ext_lower;
            }
        }

        // Если шаблон не соответствует формату "*.ext", проверяем точное совпадение
        return toLower(filename) == toLower(pattern);
    }
}

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
    std::vector<std::filesystem::path> images;
    std::filesystem::path input_path(input_dir_);

    if (!std::filesystem::exists(input_path))
    {
        Logger::error("Входная директория не существует: " + input_dir_);
        return images;
    }

    if (!std::filesystem::is_directory(input_path))
    {
        Logger::error("Указанный путь не является директорией: " + input_dir_);
        return images;
    }

    if (recursive_)
    {
        findImagesRecursive(input_path, images);
    }
    else
    {
        // Не рекурсивный поиск - только в текущей директории
        for (const auto& entry : std::filesystem::directory_iterator(input_path))
        {
            if (entry.is_regular_file() && isImageFile(entry.path()))
            {
                if (pattern_.empty() || matchesPattern(entry.path().filename().string(), pattern_))
                {
                    images.push_back(entry.path());
                }
            }
        }
    }

    return images;
}

void BatchProcessor::findImagesRecursive(const std::filesystem::path& dir,
                                        std::vector<std::filesystem::path>& images) const
{
    try
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
        {
            if (entry.is_regular_file() && isImageFile(entry.path()))
            {
                if (pattern_.empty() || matchesPattern(entry.path().filename().string(), pattern_))
                {
                    images.push_back(entry.path());
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        Logger::error("Ошибка при обходе директории: " + std::string(e.what()));
    }
}

BatchStatistics BatchProcessor::processAll(
    const std::function<bool(const std::string&, const std::string&)>& process_function,
    ProgressCallback progress_callback) const
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

    // Обрабатываем каждый файл
    for (size_t i = 0; i < images.size(); ++i)
    {
        const auto& input_file = images[i];
        std::string input_file_str = input_file.string();

        // Вызываем callback для отображения прогресса
        if (progress_callback)
        {
            progress_callback(i + 1, stats.total_files, input_file_str);
        }

        // Определяем выходной путь
        std::filesystem::path output_file;
        if (recursive_)
        {
            // Сохраняем структуру директорий
            std::filesystem::path input_path(input_dir_);
            std::filesystem::path relative = getRelativePath(input_file, input_path);
            output_file = output_path / relative;
        }
        else
        {
            // Просто копируем имя файла
            output_file = output_path / input_file.filename();
        }

        // Создаем выходную директорию для файла, если нужно
        if (!ensureOutputDirectory(output_file))
        {
            Logger::error("Не удалось создать директорию для: " + output_file.string());
            stats.failed_files++;
            continue;
        }

        std::string output_file_str = output_file.string();

        // Обрабатываем файл
        try
        {
            bool success = process_function(input_file_str, output_file_str);
            if (success)
            {
                stats.processed_files++;
                Logger::debug("Обработан: " + input_file_str + " -> " + output_file_str);
            }
            else
            {
                stats.failed_files++;
                Logger::warning("Не удалось обработать: " + input_file_str);
            }
        }
        catch (const std::exception& e)
        {
            stats.failed_files++;
            Logger::error("Ошибка при обработке " + input_file_str + ": " + e.what());
        }
    }

    return stats;
}

std::filesystem::path BatchProcessor::getRelativePath(
    const std::filesystem::path& full_path,
    const std::filesystem::path& base_dir)
{
    try
    {
        return std::filesystem::relative(full_path, base_dir);
    }
    catch (const std::filesystem::filesystem_error&)
    {
        // Если не удалось получить относительный путь, возвращаем только имя файла
        return full_path.filename();
    }
}

bool BatchProcessor::isImageFile(const std::filesystem::path& path)
{
    static const std::vector<std::string> image_extensions = {
        "jpg", "jpeg", "png"
    };
    return hasExtension(path, image_extensions);
}

bool BatchProcessor::matchesPattern(const std::string& filename, const std::string& pattern)
{
    return simplePatternMatch(filename, pattern);
}

bool BatchProcessor::ensureOutputDirectory(const std::filesystem::path& output_path) const
{
    try
    {
        std::filesystem::path parent_dir = output_path.parent_path();
        if (!parent_dir.empty())
        {
            std::filesystem::create_directories(parent_dir);
        }
        return true;
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        Logger::error("Ошибка при создании директории: " + std::string(e.what()));
        return false;
    }
}

