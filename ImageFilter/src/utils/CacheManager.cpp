#include <utils/CacheManager.h>
#include <functional>
#include <mutex>

CacheManager& CacheManager::getInstance() noexcept
{
    static CacheManager instance;
    return instance;
}

std::vector<int32_t> CacheManager::getOrGenerateKernel(
    const KernelCacheKey& key,
    const std::function<std::vector<int32_t>()>& generator)
{
    // Пытаемся получить ядро из кэша (shared lock для чтения)
    {
        std::shared_lock<std::shared_mutex> lock(kernel_cache_mutex_);
        const auto it = kernel_cache_.find(key);
        if (it != kernel_cache_.end())
        {
            return it->second;  // Возвращаем копию из кэша
        }
    }
    
    // Ядро не найдено в кэше, генерируем новое
    auto kernel = generator();
    
    // Сохраняем в кэш (exclusive lock для записи)
    {
        std::unique_lock<std::shared_mutex> lock(kernel_cache_mutex_);
        // Проверяем еще раз на случай, если другой поток уже добавил ядро
        const auto it = kernel_cache_.find(key);
        if (it == kernel_cache_.end())
        {
            kernel_cache_[key] = kernel;
        }
        else
        {
            // Используем уже существующее ядро из кэша
            kernel = it->second;
        }
    }
    
    return kernel;
}

std::vector<uint8_t> CacheManager::getOrGenerateLUT(
    const LUTCacheKey& key,
    const std::function<std::vector<uint8_t>()>& generator)
{
    // Пытаемся получить LUT из кэша (shared lock для чтения)
    {
        std::shared_lock<std::shared_mutex> lock(lut_cache_mutex_);
        const auto it = lut_cache_.find(key);
        if (it != lut_cache_.end())
        {
            return it->second;  // Возвращаем копию из кэша
        }
    }
    
    // LUT не найдена в кэше, генерируем новую
    auto lut = generator();
    
    // Сохраняем в кэш (exclusive lock для записи)
    {
        std::unique_lock<std::shared_mutex> lock(lut_cache_mutex_);
        // Проверяем еще раз на случай, если другой поток уже добавил LUT
        const auto it = lut_cache_.find(key);
        if (it == lut_cache_.end())
        {
            lut_cache_[key] = lut;
        }
        else
        {
            // Используем уже существующую LUT из кэша
            lut = it->second;
        }
    }
    
    return lut;
}

void CacheManager::clearKernelCache() noexcept
{
    std::unique_lock<std::shared_mutex> lock(kernel_cache_mutex_);
    kernel_cache_.clear();
}

void CacheManager::clearLUTCache() noexcept
{
    std::unique_lock<std::shared_mutex> lock(lut_cache_mutex_);
    lut_cache_.clear();
}

void CacheManager::clearAll() noexcept
{
    clearKernelCache();
    clearLUTCache();
}

CacheManager::CacheStatistics CacheManager::getStatistics() const noexcept
{
    CacheStatistics stats;
    
    {
        std::shared_lock<std::shared_mutex> lock(kernel_cache_mutex_);
        stats.kernel_cache_size = kernel_cache_.size();
    }
    
    {
        std::shared_lock<std::shared_mutex> lock(lut_cache_mutex_);
        stats.lut_cache_size = lut_cache_.size();
    }
    
    return stats;
}

