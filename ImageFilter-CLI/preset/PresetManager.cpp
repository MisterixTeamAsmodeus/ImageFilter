#include <preset/PresetManager.h>
#include <preset/Config.h>
#include <utils/Logger.h>
#include <cli/ImageProcessingHelper.h>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

bool PresetManager::loadPreset(const std::string& preset_file,
                               const std::string& preset_dir,
                               std::string& filter_name)
{
    Config config;
    std::string preset_path = preset_file;
    
    // Обрабатываем относительные пути
    fs::path preset_file_path(preset_file);
    if (!preset_file_path.is_absolute())
    {
        // Относительный путь - добавляем директорию пресетов
        fs::path full_path = fs::path(preset_dir) / preset_file_path;
        
        // Добавляем расширение .json, если его нет
        if (full_path.extension() != ".json")
        {
            full_path.replace_extension(".json");
        }
        
        preset_path = full_path.string();
    }
    
    auto preset = config.loadPreset(preset_path);
    if (!preset)
    {
        Logger::error("Ошибка: не удалось загрузить пресет: " + preset_path);
        return false;
    }
    
    if (!config.validateConfig(*preset))
    {
        Logger::error("Ошибка: пресет содержит невалидную конфигурацию");
        return false;
    }
    
    // Преобразуем пресет в цепочку фильтров
    filter_name.clear();
    for (size_t i = 0; i < preset->filters.size(); ++i)
    {
        if (i > 0)
        {
            filter_name += ",";
        }
        filter_name += preset->filters[i].name;
    }
    
    Logger::info("Загружен пресет: " + preset->name);
    if (!preset->description.empty())
    {
        Logger::info("Описание: " + preset->description);
    }
    
    return true;
}

bool PresetManager::savePreset(const std::string& filter_name,
                               const std::string& preset_name,
                               const std::string& preset_dir)
{
    auto filters = ImageProcessingHelper::parseFilterChain(filter_name);
    if (filters.empty())
    {
        Logger::error("Ошибка: не указаны фильтры для сохранения в пресет");
        return false;
    }
    
    Config config;
    FilterChainConfig preset_config;
    preset_config.name = preset_name;
    preset_config.description = "Сохраненный пресет: " + filter_name;
    
    // Преобразуем цепочку фильтров в конфигурацию
    for (const auto& filter_name_str : filters)
    {
        FilterConfig filter_cfg;
        filter_cfg.name = filter_name_str;
        preset_config.filters.push_back(filter_cfg);
    }
    
    // Используем кроссплатформенные пути
    fs::path preset_path = fs::path(preset_dir) / (preset_name + ".json");
    if (config.savePreset(preset_path.string(), preset_config))
    {
        Logger::info("Пресет сохранен: " + preset_path.string());
        return true;
    }
    else
    {
        Logger::error("Ошибка: не удалось сохранить пресет: " + preset_path.string());
        return false;
    }
}

