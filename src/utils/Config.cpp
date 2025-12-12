#include <utils/Config.h>
#include <utils/FilterFactory.h>
#include <utils/PathValidator.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

using json = nlohmann::json;

bool Config::savePreset(const std::string& filepath, const FilterChainConfig& config) const
{
    try
    {
        // Валидация пути
        if (PathValidator::containsDangerousCharacters(filepath))
        {
            return false;
        }

        const std::string normalized_path = PathValidator::normalizeAndValidate(filepath);
        if (normalized_path.empty())
        {
            return false;
        }

        // Создаем директорию, если она не существует
        fs::path path(normalized_path);
        if (path.has_parent_path())
        {
            fs::create_directories(path.parent_path());
        }

        // Сериализуем конфигурацию в JSON
        json j;
        j["name"] = config.name;
        j["description"] = config.description;
        j["filters"] = json::array();
        
        for (const auto& filter : config.filters)
        {
            json filter_obj;
            filter_obj["name"] = filter.name;
            filter_obj["params"] = filter.params;
            j["filters"].push_back(filter_obj);
        }
        
        const std::string json_str = j.dump(2);
        
        // Проверяем размер конфигурации перед сохранением
        if (json_str.size() > PathValidator::DEFAULT_MAX_CONFIG_SIZE)
        {
            return false;
        }

        // Открываем файл для записи
        std::ofstream file(normalized_path);
        if (!file.is_open())
        {
            return false;
        }

        // Записываем JSON
        file << json_str;
        
        return file.good();
    }
    catch (...)
    {
        return false;
    }
}

std::unique_ptr<FilterChainConfig> Config::loadPreset(const std::string& filepath) const
{
    try
    {
        // Валидация пути
        if (PathValidator::containsDangerousCharacters(filepath))
        {
            return nullptr;
        }

        const std::string normalized_path = PathValidator::normalizeAndValidate(filepath);
        if (normalized_path.empty())
        {
            return nullptr;
        }

        // Проверяем существование файла
        if (!fs::exists(normalized_path))
        {
            return nullptr;
        }

        // Проверяем размер файла
        if (!PathValidator::validateFileSize(normalized_path, PathValidator::DEFAULT_MAX_CONFIG_SIZE))
        {
            return nullptr;
        }

        // Читаем файл
        std::ifstream file(normalized_path);
        if (!file.is_open())
        {
            return nullptr;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        const std::string json_str = buffer.str();

        // Парсим JSON
        try
        {
            const json j = json::parse(json_str);
            
            auto config = std::make_unique<FilterChainConfig>();
            config->name = j.value("name", "");
            config->description = j.value("description", "");
            
            if (j.contains("filters") && j["filters"].is_array())
            {
                for (const auto& filter_obj : j["filters"])
                {
                    FilterConfig filter;
                    filter.name = filter_obj.value("name", "");
                    
                    if (filter_obj.contains("params") && filter_obj["params"].is_object())
                    {
                        for (const auto& [key, value] : filter_obj["params"].items())
                        {
                            if (value.is_string())
                            {
                                filter.params[key] = value.get<std::string>();
                            }
                            else if (value.is_number())
                            {
                                filter.params[key] = std::to_string(value.get<double>());
                            }
                            else
                            {
                                filter.params[key] = value.dump();
                            }
                        }
                    }
                    
                    if (!filter.name.empty())
                    {
                        config->filters.push_back(filter);
                    }
                }
            }
            
            return config;
        }
        catch (const json::exception&)
        {
            return nullptr;
        }
    }
    catch (...)
    {
        return nullptr;
    }
}

bool Config::validateConfig(const FilterChainConfig& config) const
{
    // Проверяем, что имя не пустое
    if (config.name.empty())
    {
        return false;
    }

    // Проверяем, что все фильтры зарегистрированы
    auto& factory = FilterFactory::getInstance();
    for (const auto& filter : config.filters)
    {
        if (filter.name.empty())
        {
            return false;
        }

        if (!factory.isRegistered(filter.name))
        {
            return false;
        }
    }

    return true;
}

std::vector<std::string> Config::listPresets(const std::string& directory) const
{
    std::vector<std::string> presets;

    try
    {
        if (!fs::exists(directory) || !fs::is_directory(directory))
        {
            return presets;
        }

        for (const auto& entry : fs::directory_iterator(directory))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                presets.push_back(entry.path().stem().string());
            }
        }

        std::sort(presets.begin(), presets.end());
    }
    catch (...)
    {
        // В случае ошибки возвращаем пустой список
    }

    return presets;
}

bool Config::deletePreset(const std::string& filepath) const
{
    try
    {
        if (!fs::exists(filepath))
        {
            return false;
        }

        return fs::remove(filepath);
    }
    catch (...)
    {
        return false;
    }
}


