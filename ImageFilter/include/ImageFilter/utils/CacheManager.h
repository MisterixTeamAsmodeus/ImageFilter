#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include <functional>

/**
 * @brief Ключ для кэша ядер свертки
 */
struct KernelCacheKey
{
    enum class Type
    {
        Gaussian,
        BoxBlur
    };
    
    Type type;
    double radius;
    double sigma;  // Для Gaussian
    
    /**
     * @brief Оператор сравнения для использования в unordered_map
     */
    bool operator==(const KernelCacheKey& other) const noexcept
    {
        if (type != other.type)
        {
            return false;
        }
        if (type == Type::Gaussian)
        {
            return radius == other.radius && sigma == other.sigma;
        }
        else
        {
            return radius == other.radius;
        }
    }
};

/**
 * @brief Хэш-функция для KernelCacheKey
 */
struct KernelCacheKeyHash
{
    std::size_t operator()(const KernelCacheKey& key) const noexcept
    {
        std::size_t h1 = std::hash<int>{}(static_cast<int>(key.type));
        std::size_t h2 = std::hash<double>{}(key.radius);
        std::size_t h3 = std::hash<double>{}(key.sigma);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

/**
 * @brief Ключ для кэша LUT преобразований
 */
struct LUTCacheKey
{
    enum class Type
    {
        Gamma,
        Brightness,
        Contrast
    };
    
    Type type;
    double parameter;  // gamma, brightness или contrast значение
    
    /**
     * @brief Оператор сравнения для использования в unordered_map
     */
    bool operator==(const LUTCacheKey& other) const noexcept
    {
        return type == other.type && parameter == other.parameter;
    }
};

/**
 * @brief Хэш-функция для LUTCacheKey
 */
struct LUTCacheKeyHash
{
    std::size_t operator()(const LUTCacheKey& key) const noexcept
    {
        std::size_t h1 = std::hash<int>{}(static_cast<int>(key.type));
        std::size_t h2 = std::hash<double>{}(key.parameter);
        return h1 ^ (h2 << 1);
    }
};

/**
 * @brief Централизованный менеджер кэша для оптимизации производительности
 * 
 * Управляет кэшированием:
 * - Ядер свертки (Gaussian, Box Blur)
 * - LUT таблиц для преобразований (гамма-коррекция, яркость, контраст)
 * 
 * Thread-safe: все операции синхронизированы с использованием shared_mutex
 */
class CacheManager
{
public:
    /**
     * @brief Получает единственный экземпляр CacheManager (Singleton)
     * @return Ссылка на CacheManager
     */
    static CacheManager& getInstance() noexcept;
    
    /**
     * @brief Получает или генерирует ядро свертки из кэша
     * @param key Ключ кэша (тип, радиус, sigma)
     * @param generator Функция генерации ядра, если его нет в кэше
     * @return Вектор коэффициентов ядра (масштабированные на 65536)
     */
    std::vector<int32_t> getOrGenerateKernel(
        const KernelCacheKey& key,
        const std::function<std::vector<int32_t>()>& generator);
    
    /**
     * @brief Получает или генерирует LUT таблицу из кэша
     * @param key Ключ кэша (тип, параметр)
     * @param generator Функция генерации LUT, если её нет в кэше
     * @return Вектор значений LUT (256 элементов для 8-битных значений)
     */
    std::vector<uint8_t> getOrGenerateLUT(
        const LUTCacheKey& key,
        const std::function<std::vector<uint8_t>()>& generator);
    
    /**
     * @brief Очищает кэш ядер свертки
     */
    void clearKernelCache() noexcept;
    
    /**
     * @brief Очищает кэш LUT таблиц
     */
    void clearLUTCache() noexcept;
    
    /**
     * @brief Очищает все кэши
     */
    void clearAll() noexcept;
    
    /**
     * @brief Получает статистику использования кэша
     * @return Структура со статистикой
     */
    struct CacheStatistics
    {
        size_t kernel_cache_size = 0;
        size_t lut_cache_size = 0;
    };
    
    CacheStatistics getStatistics() const noexcept;

private:
    CacheManager() = default;
    ~CacheManager() = default;
    
    // Запрещаем копирование и присваивание
    CacheManager(const CacheManager&) = delete;
    CacheManager& operator=(const CacheManager&) = delete;
    
    // Кэш ядер свертки
    mutable std::shared_mutex kernel_cache_mutex_;
    std::unordered_map<KernelCacheKey, std::vector<int32_t>, KernelCacheKeyHash> kernel_cache_;
    
    // Кэш LUT таблиц
    mutable std::shared_mutex lut_cache_mutex_;
    std::unordered_map<LUTCacheKey, std::vector<uint8_t>, LUTCacheKeyHash> lut_cache_;
};

