#include <utils/FileSystemHelper.h>
#include <utils/Logger.h>
#include <algorithm>
#include <cctype>
#include <filesystem>

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

std::vector<std::filesystem::path> FileSystemHelper::findImages(
    const std::string& input_dir,
    bool recursive,
    const std::string& pattern)
{
    std::vector<std::filesystem::path> images;
    std::filesystem::path input_path(input_dir);

    if (!std::filesystem::exists(input_path))
    {
        Logger::error("Входная директория не существует: " + input_dir);
        return images;
    }

    if (!std::filesystem::is_directory(input_path))
    {
        Logger::error("Указанный путь не является директорией: " + input_dir);
        return images;
    }

    if (recursive)
    {
        findImagesRecursive(input_path, images, pattern);
    }
    else
    {
        // Не рекурсивный поиск - только в текущей директории
        for (const auto& entry : std::filesystem::directory_iterator(input_path))
        {
            if (entry.is_regular_file() && isImageFile(entry.path()))
            {
                if (pattern.empty() || matchesPattern(entry.path().filename().string(), pattern))
                {
                    images.push_back(entry.path());
                }
            }
        }
    }

    return images;
}

void FileSystemHelper::findImagesRecursive(
    const std::filesystem::path& dir,
    std::vector<std::filesystem::path>& images,
    const std::string& pattern)
{
    try
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
        {
            if (entry.is_regular_file() && isImageFile(entry.path()))
            {
                if (pattern.empty() || matchesPattern(entry.path().filename().string(), pattern))
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

bool FileSystemHelper::isImageFile(const std::filesystem::path& path)
{
    static const std::vector<std::string> image_extensions = {
        "jpg", "jpeg", "png"
    };
    return hasExtension(path, image_extensions);
}

bool FileSystemHelper::matchesPattern(const std::string& filename, const std::string& pattern)
{
    return simplePatternMatch(filename, pattern);
}

std::filesystem::path FileSystemHelper::getRelativePath(
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

bool FileSystemHelper::ensureOutputDirectory(const std::filesystem::path& output_path)
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

