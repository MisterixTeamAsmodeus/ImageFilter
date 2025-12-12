#include <utils/ResumeStateManager.h>
#include <utils/Logger.h>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <algorithm>

namespace
{
    /**
     * @brief Убирает пробелы в начале и конце строки
     */
    std::string trim(const std::string& str)
    {
        if (str.empty())
        {
            return str;
        }
        
        size_t start = str.find_first_not_of(" \t");
        if (start == std::string::npos)
        {
            return "";
        }
        
        size_t end = str.find_last_not_of(" \t");
        return str.substr(start, end - start + 1);
    }
}

std::set<std::string> ResumeStateManager::loadResumeState(const std::string& state_file)
{
    std::set<std::string> processed_files;

    if (state_file.empty())
    {
        return processed_files;
    }

    try
    {
        if (!std::filesystem::exists(state_file))
        {
            return processed_files;
        }

        std::ifstream file(state_file);
        if (!file.is_open())
        {
            Logger::warning("Не удалось открыть файл состояния: " + state_file);
            return processed_files;
        }

        std::string line;
        while (std::getline(file, line))
        {
            line = trim(line);
            if (!line.empty() && line[0] != '#')  // Игнорируем пустые строки и комментарии
            {
                processed_files.insert(line);
            }
        }
    }
    catch (const std::exception& e)
    {
        Logger::warning("Ошибка при загрузке состояния возобновления: " + std::string(e.what()));
    }

    return processed_files;
}

bool ResumeStateManager::saveResumeState(const std::string& state_file, const std::set<std::string>& processed_files)
{
    if (state_file.empty())
    {
        return false;
    }

    try
    {
        // Создаем директорию, если нужно
        std::filesystem::path path(state_file);
        if (path.has_parent_path())
        {
            std::filesystem::create_directories(path.parent_path());
        }

        std::ofstream file(state_file);
        if (!file.is_open())
        {
            Logger::warning("Не удалось открыть файл состояния для записи: " + state_file);
            return false;
        }

        // Записываем заголовок
        file << "# Состояние возобновления пакетной обработки\n";
        file << "# Каждая строка содержит путь к обработанному файлу\n";

        // Записываем все обработанные файлы
        for (const auto& file_path : processed_files)
        {
            file << file_path << "\n";
        }

        return file.good();
    }
    catch (const std::exception& e)
    {
        Logger::warning("Ошибка при сохранении состояния возобновления: " + std::string(e.what()));
        return false;
    }
}

bool ResumeStateManager::isFileProcessed(const std::filesystem::path& output_path)
{
    try
    {
        return std::filesystem::exists(output_path) && std::filesystem::is_regular_file(output_path);
    }
    catch (const std::filesystem::filesystem_error&)
    {
        return false;
    }
}

