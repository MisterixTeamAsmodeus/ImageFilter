#include <utils/Config.h>
#include <utils/FilterFactory.h>
#include <utils/PathValidator.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

namespace
{
    /**
     * @brief Удаляет пробелы в начале и конце строки
     */
    std::string trim(const std::string& str)
    {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos)
        {
            return "";
        }
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }

    /**
     * @brief Экранирует специальные символы в JSON
     */
    std::string escapeJson(const std::string& str)
    {
        std::string result;
        result.reserve(str.length() + 10);
        
        for (char c : str)
        {
            switch (c)
            {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c; break;
            }
        }
        
        return result;
    }

    /**
     * @brief Проверяет, является ли строка числом
     */
    bool isNumber(const std::string& str)
    {
        if (str.empty())
        {
            return false;
        }
        
        size_t start = 0;
        if (str[0] == '-' || str[0] == '+')
        {
            start = 1;
        }
        
        if (start >= str.length())
        {
            return false;
        }
        
        bool has_dot = false;
        for (size_t i = start; i < str.length(); ++i)
        {
            if (str[i] == '.')
            {
                if (has_dot)
                {
                    return false;
                }
                has_dot = true;
            }
            else if (!std::isdigit(static_cast<unsigned char>(str[i])))
            {
                return false;
            }
        }
        
        return true;
    }
}

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

        // Проверяем размер конфигурации перед сохранением
        const std::string json = toJson(config);
        if (json.size() > PathValidator::DEFAULT_MAX_CONFIG_SIZE)
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
        file << toJson(config);
        
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
        std::string json = buffer.str();

        // Парсим JSON
        return parseJson(json);
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

std::unique_ptr<FilterChainConfig> Config::parseJson(const std::string& json) const
{
    // Простой парсер JSON для базовых случаев
    // Для более сложных случаев можно использовать библиотеку (например, nlohmann/json)
    
    auto config = std::make_unique<FilterChainConfig>();
    
    // Ищем поля name и description
    size_t name_pos = json.find("\"name\"");
    if (name_pos != std::string::npos)
    {
        size_t colon_pos = json.find(':', name_pos);
        size_t quote_start = json.find('"', colon_pos);
        if (quote_start != std::string::npos)
        {
            size_t quote_end = json.find('"', quote_start + 1);
            if (quote_end != std::string::npos)
            {
                config->name = json.substr(quote_start + 1, quote_end - quote_start - 1);
            }
        }
    }

    size_t desc_pos = json.find("\"description\"");
    if (desc_pos != std::string::npos)
    {
        size_t colon_pos = json.find(':', desc_pos);
        size_t quote_start = json.find('"', colon_pos);
        if (quote_start != std::string::npos)
        {
            size_t quote_end = json.find('"', quote_start + 1);
            if (quote_end != std::string::npos)
            {
                config->description = json.substr(quote_start + 1, quote_end - quote_start - 1);
            }
        }
    }

    // Парсим массив фильтров
    size_t filters_pos = json.find("\"filters\"");
    if (filters_pos != std::string::npos)
    {
        size_t array_start = json.find('[', filters_pos);
        if (array_start != std::string::npos)
        {
            size_t array_end = json.find(']', array_start);
            if (array_end != std::string::npos)
            {
                std::string filters_array = json.substr(array_start + 1, array_end - array_start - 1);
                
                // Простой парсинг объектов фильтров
                size_t obj_start = 0;
                while ((obj_start = filters_array.find('{', obj_start)) != std::string::npos)
                {
                    size_t obj_end = filters_array.find('}', obj_start);
                    if (obj_end == std::string::npos)
                    {
                        break;
                    }

                    std::string filter_obj = filters_array.substr(obj_start, obj_end - obj_start + 1);
                    
                    FilterConfig filter;
                    
                    // Ищем имя фильтра
                    size_t name_pos_in_obj = filter_obj.find("\"name\"");
                    if (name_pos_in_obj != std::string::npos)
                    {
                        size_t colon_pos = filter_obj.find(':', name_pos_in_obj);
                        size_t quote_start = filter_obj.find('"', colon_pos);
                        if (quote_start != std::string::npos)
                        {
                            size_t quote_end = filter_obj.find('"', quote_start + 1);
                            if (quote_end != std::string::npos)
                            {
                                filter.name = filter_obj.substr(quote_start + 1, quote_end - quote_start - 1);
                            }
                        }
                    }

                    // Ищем параметры
                    size_t params_pos = filter_obj.find("\"params\"");
                    if (params_pos != std::string::npos)
                    {
                        size_t params_start = filter_obj.find('{', params_pos);
                        if (params_start != std::string::npos)
                        {
                            size_t params_end = filter_obj.find('}', params_start);
                            if (params_end != std::string::npos)
                            {
                                std::string params_str = filter_obj.substr(params_start + 1, params_end - params_start - 1);
                                
                                // Простой парсинг ключ-значение
                                size_t key_start = 0;
                                while ((key_start = params_str.find('"', key_start)) != std::string::npos)
                                {
                                    size_t key_end = params_str.find('"', key_start + 1);
                                    if (key_end == std::string::npos)
                                    {
                                        break;
                                    }
                                    
                                    std::string key = params_str.substr(key_start + 1, key_end - key_start - 1);
                                    
                                    size_t colon_pos = params_str.find(':', key_end);
                                    if (colon_pos == std::string::npos)
                                    {
                                        break;
                                    }
                                    
                                    size_t value_start = params_str.find_first_not_of(" \t", colon_pos + 1);
                                    if (value_start == std::string::npos)
                                    {
                                        break;
                                    }
                                    
                                    size_t value_end;
                                    if (params_str[value_start] == '"')
                                    {
                                        value_start++;
                                        value_end = params_str.find('"', value_start);
                                        if (value_end == std::string::npos)
                                        {
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        value_end = params_str.find_first_of(",}", value_start);
                                        if (value_end == std::string::npos)
                                        {
                                            value_end = params_str.length();
                                        }
                                    }
                                    
                                    std::string value = trim(params_str.substr(value_start, value_end - value_start));
                                    if (value.front() == '"' && value.back() == '"')
                                    {
                                        value = value.substr(1, value.length() - 2);
                                    }
                                    
                                    filter.params[key] = value;
                                    key_start = value_end + 1;
                                }
                            }
                        }
                    }

                    config->filters.push_back(filter);
                    obj_start = obj_end + 1;
                }
            }
        }
    }

    return config;
}

std::string Config::toJson(const FilterChainConfig& config) const
{
    std::ostringstream json;
    json << "{\n";
    json << "  \"name\": \"" << escapeJson(config.name) << "\",\n";
    json << "  \"description\": \"" << escapeJson(config.description) << "\",\n";
    json << "  \"filters\": [\n";
    
    for (size_t i = 0; i < config.filters.size(); ++i)
    {
        const auto& filter = config.filters[i];
        json << "    {\n";
        json << "      \"name\": \"" << escapeJson(filter.name) << "\",\n";
        json << "      \"params\": {\n";
        
        size_t param_index = 0;
        for (const auto& [key, value] : filter.params)
        {
            json << "        \"" << escapeJson(key) << "\": ";
            
            // Определяем, нужно ли кавычки (для строк)
            if (isNumber(value))
            {
                json << value;
            }
            else
            {
                json << "\"" << escapeJson(value) << "\"";
            }
            
            if (param_index < filter.params.size() - 1)
            {
                json << ",";
            }
            json << "\n";
            param_index++;
        }
        
        json << "      }\n";
        json << "    }";
        
        if (i < config.filters.size() - 1)
        {
            json << ",";
        }
        json << "\n";
    }
    
    json << "  ]\n";
    json << "}\n";
    
    return json.str();
}

