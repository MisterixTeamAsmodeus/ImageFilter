#pragma once

#include <filters/IFilter.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

// Forward declaration
namespace CLI {
    class App;
}

class IBufferPool;

/**
 * @brief Фабрика для создания фильтров изображений
 * 
 * Реализует паттерн Factory для создания фильтров по имени.
 * Использует регистрацию фильтров через map для избежания больших if-else блоков.
 * Поддерживает создание фильтров с параметрами из CLI::App.
 * 
 * @example example_filter_usage.cpp
 * Пример использования FilterFactory:
 * @code{.cpp}
 * FilterFactory::getInstance().registerAll();
 * auto filter = FilterFactory::getInstance().create("grayscale", app);
 * @endcode
 */
class FilterFactory
{
public:
    /**
     * @brief Тип функции для создания фильтра
     */
    using FilterCreator = std::function<std::unique_ptr<IFilter>(const CLI::App&)>;

    /**
     * @brief Получает единственный экземпляр фабрики (Singleton)
     * @return Ссылка на экземпляр FilterFactory
     */
    static FilterFactory& getInstance();

    /**
     * @brief Регистрирует все доступные фильтры
     * 
     * Должен быть вызван один раз при инициализации приложения.
     */
    void registerAll();

    /**
     * @brief Регистрирует фильтр с заданным именем
     * @param name Имя фильтра
     * @param creator Функция создания фильтра
     */
    void registerFilter(const std::string& name, FilterCreator creator);

    /**
     * @brief Создает фильтр по имени
     * @param name Имя фильтра
     * @param app CLI::App для получения параметров фильтра
     * @return Умный указатель на фильтр или nullptr если фильтр не найден
     */
    std::unique_ptr<IFilter> create(const std::string& name, const CLI::App& app) const;

    /**
     * @brief Проверяет, зарегистрирован ли фильтр с заданным именем
     * @param name Имя фильтра
     * @return true если фильтр зарегистрирован
     */
    bool isRegistered(const std::string& name) const;

    /**
     * @brief Получает список всех зарегистрированных фильтров
     * @return Вектор имен фильтров
     */
    std::vector<std::string> getRegisteredFilters() const;

    /**
     * @brief Устанавливает пул буферов для использования при создании фильтров
     * 
     * Пул буферов будет передаваться фильтрам, которые его поддерживают.
     * Это позволяет оптимизировать использование памяти при обработке цепочек фильтров.
     * 
     * @param buffer_pool Указатель на пул буферов (может быть nullptr)
     */
    void setBufferPool(IBufferPool* buffer_pool) noexcept;

    /**
     * @brief Получает текущий пул буферов
     * @return Указатель на пул буферов или nullptr
     */
    [[nodiscard]] IBufferPool* getBufferPool() const noexcept;

private:
    /**
     * @brief Приватный конструктор для Singleton
     */
    FilterFactory() = default;

    /**
     * @brief Запрещаем копирование
     */
    FilterFactory(const FilterFactory&) = delete;
    FilterFactory& operator=(const FilterFactory&) = delete;

    std::unordered_map<std::string, FilterCreator> creators_;  // Map имен фильтров на функции создания
    IBufferPool* buffer_pool_ = nullptr;  // Пул буферов для переиспользования (опционально)
};
