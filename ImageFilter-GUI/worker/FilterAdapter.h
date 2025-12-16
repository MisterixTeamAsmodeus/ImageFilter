#pragma once

#include <filters/IFilter.h>
#include <utils/BufferPool.h>
#include <memory>
#include <string>
#include <map>
#include <QVariant>

/**
 * @brief Адаптер для создания фильтров без CLI::App
 *
 * Предоставляет функции для создания фильтров напрямую через конструкторы
 * на основе имени фильтра и его параметров.
 */
namespace FilterAdapter
{
    /**
     * @brief Создает фильтр по имени и параметрам
     * @param filterName Имя фильтра
     * @param parameters Параметры фильтра
     * @param bufferPool Пул буферов для оптимизации (опционально)
     * @return Умный указатель на фильтр или nullptr, если фильтр не найден
     */
    std::unique_ptr<IFilter> createFilter(
        const std::string& filterName,
        const std::map<std::string, QVariant>& parameters = {},
        IBufferPool* bufferPool = nullptr);

    /**
     * @brief Получает значение параметра как double
     * @param parameters Карта параметров
     * @param paramName Имя параметра
     * @param defaultValue Значение по умолчанию
     * @return Значение параметра или значение по умолчанию
     */
    double getDoubleParameter(const std::map<std::string, QVariant>& parameters,
                             const std::string& paramName,
                             double defaultValue);

    /**
     * @brief Получает значение параметра как int
     * @param parameters Карта параметров
     * @param paramName Имя параметра
     * @param defaultValue Значение по умолчанию
     * @return Значение параметра или значение по умолчанию
     */
    int getIntParameter(const std::map<std::string, QVariant>& parameters,
                       const std::string& paramName,
                       int defaultValue);

    /**
     * @brief Получает значение параметра как bool
     * @param parameters Карта параметров
     * @param paramName Имя параметра
     * @param defaultValue Значение по умолчанию
     * @return Значение параметра или значение по умолчанию
     */
    bool getBoolParameter(const std::map<std::string, QVariant>& parameters,
                         const std::string& paramName,
                         bool defaultValue);

    /**
     * @brief Получает значение параметра как строку
     * @param parameters Карта параметров
     * @param paramName Имя параметра
     * @param defaultValue Значение по умолчанию
     * @return Значение параметра или значение по умолчанию
     */
    std::string getStringParameter(const std::map<std::string, QVariant>& parameters,
                                   const std::string& paramName,
                                   const std::string& defaultValue);
}

