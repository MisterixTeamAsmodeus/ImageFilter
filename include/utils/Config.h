#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

/**
 * @brief Структура для хранения параметров фильтра
 */
struct FilterConfig
{
    std::string name;                          ///< Имя фильтра
    std::map<std::string, std::string> params; ///< Параметры фильтра (ключ-значение)
};

/**
 * @brief Структура для хранения конфигурации цепочки фильтров
 */
struct FilterChainConfig
{
    std::string name;                          ///< Имя пресета
    std::string description;                   ///< Описание пресета
    std::vector<FilterConfig> filters;         ///< Цепочка фильтров
};

/**
 * @brief Класс для работы с конфигурационными файлами фильтров
 * 
 * Предоставляет функциональность для сохранения и загрузки пресетов фильтров
 * в формате JSON. Позволяет сохранять часто используемые цепочки фильтров
 * для последующего переиспользования.
 * 
 * Пример использования:
 * @code
 * Config config;
 * 
 * // Создаем конфигурацию цепочки фильтров
 * FilterChainConfig chain;
 * chain.name = "vintage";
 * chain.description = "Винтажный эффект";
 * chain.filters.push_back({"grayscale", {}});
 * chain.filters.push_back({"sepia", {}});
 * chain.filters.push_back({"vignette", {{"strength", "0.5"}}});
 * 
 * // Сохраняем конфигурацию
 * config.savePreset("vintage.json", chain);
 * 
 * // Загружаем конфигурацию
 * auto loaded = config.loadPreset("vintage.json");
 * @endcode
 */
class Config
{
public:
    /**
     * @brief Конструктор по умолчанию
     */
    Config() = default;

    /**
     * @brief Деструктор
     */
    ~Config() = default;

    // Запрещаем копирование
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    /**
     * @brief Сохраняет пресет фильтров в файл
     * 
     * Сохраняет конфигурацию цепочки фильтров в JSON файл.
     * 
     * @param filepath Путь к файлу для сохранения
     * @param config Конфигурация цепочки фильтров
     * @return true если сохранение успешно, false в противном случае
     */
    bool savePreset(const std::string& filepath, const FilterChainConfig& config) const;

    /**
     * @brief Загружает пресет фильтров из файла
     * 
     * Загружает конфигурацию цепочки фильтров из JSON файла.
     * 
     * @param filepath Путь к файлу для загрузки
     * @return Умный указатель на конфигурацию или nullptr при ошибке
     */
    std::unique_ptr<FilterChainConfig> loadPreset(const std::string& filepath) const;

    /**
     * @brief Валидирует конфигурацию цепочки фильтров
     * 
     * Проверяет, что все фильтры в цепочке зарегистрированы и параметры корректны.
     * 
     * @param config Конфигурация для валидации
     * @return true если конфигурация валидна, false в противном случае
     */
    bool validateConfig(const FilterChainConfig& config) const;

    /**
     * @brief Получает список всех сохраненных пресетов в директории
     * 
     * @param directory Директория для поиска пресетов
     * @return Вектор имен пресетов (без расширения .json)
     */
    std::vector<std::string> listPresets(const std::string& directory) const;

    /**
     * @brief Удаляет пресет
     * 
     * @param filepath Путь к файлу пресета
     * @return true если удаление успешно, false в противном случае
     */
    bool deletePreset(const std::string& filepath) const;

private:
    /**
     * @brief Парсит JSON строку в FilterChainConfig
     * 
     * @param json JSON строка
     * @return Умный указатель на конфигурацию или nullptr при ошибке
     */
    std::unique_ptr<FilterChainConfig> parseJson(const std::string& json) const;

    /**
     * @brief Преобразует FilterChainConfig в JSON строку
     * 
     * @param config Конфигурация для преобразования
     * @return JSON строка
     */
    std::string toJson(const FilterChainConfig& config) const;
};

