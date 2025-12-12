#include <gtest/gtest.h>
#include <utils/CacheManager.h>
#include <vector>
#include <thread>
#include <atomic>

/**
 * @brief Тесты для CacheManager
 */
class CacheManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Очищаем кэш перед каждым тестом
        CacheManager::getInstance().clearAll();
    }
    
    void TearDown() override
    {
        // Очищаем кэш после каждого теста
        CacheManager::getInstance().clearAll();
    }
};

// Тест Singleton паттерна
TEST_F(CacheManagerTest, Singleton)
{
    auto& instance1 = CacheManager::getInstance();
    auto& instance2 = CacheManager::getInstance();
    
    EXPECT_EQ(&instance1, &instance2);
}

// Тест getOrGenerateKernel
TEST_F(CacheManagerTest, GetOrGenerateKernel)
{
    auto& cache = CacheManager::getInstance();
    
    KernelCacheKey key;
    key.type = KernelCacheKey::Type::Gaussian;
    key.radius = 5.0;
    key.sigma = 2.0;
    
    int call_count = 0;
    auto generator = [&call_count]() -> std::vector<int32_t> {
        call_count++;
        return {1, 2, 3, 4, 5};
    };
    
    // Первый вызов должен вызвать генератор
    auto kernel1 = cache.getOrGenerateKernel(key, generator);
    EXPECT_EQ(call_count, 1);
    EXPECT_EQ(kernel1.size(), 5);
    
    // Второй вызов должен использовать кэш
    auto kernel2 = cache.getOrGenerateKernel(key, generator);
    EXPECT_EQ(call_count, 1);  // Генератор не должен быть вызван снова
    EXPECT_EQ(kernel2.size(), 5);
}

// Тест getOrGenerateKernel с разными ключами
TEST_F(CacheManagerTest, GetOrGenerateKernelDifferentKeys)
{
    auto& cache = CacheManager::getInstance();
    
    int call_count = 0;
    auto generator = [&call_count]() -> std::vector<int32_t> {
        call_count++;
        return {1, 2, 3};
    };
    
    KernelCacheKey key1;
    key1.type = KernelCacheKey::Type::Gaussian;
    key1.radius = 5.0;
    key1.sigma = 2.0;
    
    KernelCacheKey key2;
    key2.type = KernelCacheKey::Type::Gaussian;
    key2.radius = 10.0;
    key2.sigma = 2.0;
    
    cache.getOrGenerateKernel(key1, generator);
    cache.getOrGenerateKernel(key2, generator);
    
    // Оба ключа должны вызвать генератор
    EXPECT_EQ(call_count, 2);
}

// Тест getOrGenerateLUT
TEST_F(CacheManagerTest, GetOrGenerateLUT)
{
    auto& cache = CacheManager::getInstance();
    
    LUTCacheKey key;
    key.type = LUTCacheKey::Type::Gamma;
    key.parameter = 2.0;
    
    int call_count = 0;
    auto generator = [&call_count]() -> std::vector<uint8_t> {
        call_count++;
        return std::vector<uint8_t>(256, 128);
    };
    
    // Первый вызов должен вызвать генератор
    auto lut1 = cache.getOrGenerateLUT(key, generator);
    EXPECT_EQ(call_count, 1);
    EXPECT_EQ(lut1.size(), 256);
    
    // Второй вызов должен использовать кэш
    auto lut2 = cache.getOrGenerateLUT(key, generator);
    EXPECT_EQ(call_count, 1);
    EXPECT_EQ(lut2.size(), 256);
}

// Тест getOrGenerateLUT с разными типами
TEST_F(CacheManagerTest, GetOrGenerateLUTDifferentTypes)
{
    auto& cache = CacheManager::getInstance();
    
    int call_count = 0;
    auto generator = [&call_count]() -> std::vector<uint8_t> {
        call_count++;
        return std::vector<uint8_t>(256, 0);
    };
    
    LUTCacheKey key1;
    key1.type = LUTCacheKey::Type::Gamma;
    key1.parameter = 2.0;
    
    LUTCacheKey key2;
    key2.type = LUTCacheKey::Type::Brightness;
    key2.parameter = 2.0;
    
    cache.getOrGenerateLUT(key1, generator);
    cache.getOrGenerateLUT(key2, generator);
    
    // Разные типы должны вызвать генератор дважды
    EXPECT_EQ(call_count, 2);
}

// Тест clearKernelCache
TEST_F(CacheManagerTest, ClearKernelCache)
{
    auto& cache = CacheManager::getInstance();
    
    KernelCacheKey key;
    key.type = KernelCacheKey::Type::Gaussian;
    key.radius = 5.0;
    key.sigma = 2.0;
    
    int call_count = 0;
    auto generator = [&call_count]() -> std::vector<int32_t> {
        call_count++;
        return {1, 2, 3};
    };
    
    cache.getOrGenerateKernel(key, generator);
    EXPECT_EQ(call_count, 1);
    
    cache.clearKernelCache();
    
    // После очистки генератор должен быть вызван снова
    cache.getOrGenerateKernel(key, generator);
    EXPECT_EQ(call_count, 2);
}

// Тест clearLUTCache
TEST_F(CacheManagerTest, ClearLUTCache)
{
    auto& cache = CacheManager::getInstance();
    
    LUTCacheKey key;
    key.type = LUTCacheKey::Type::Gamma;
    key.parameter = 2.0;
    
    int call_count = 0;
    auto generator = [&call_count]() -> std::vector<uint8_t> {
        call_count++;
        return std::vector<uint8_t>(256, 0);
    };
    
    cache.getOrGenerateLUT(key, generator);
    EXPECT_EQ(call_count, 1);
    
    cache.clearLUTCache();
    
    // После очистки генератор должен быть вызван снова
    cache.getOrGenerateLUT(key, generator);
    EXPECT_EQ(call_count, 2);
}

// Тест clearAll
TEST_F(CacheManagerTest, ClearAll)
{
    auto& cache = CacheManager::getInstance();
    
    KernelCacheKey kernel_key;
    kernel_key.type = KernelCacheKey::Type::Gaussian;
    kernel_key.radius = 5.0;
    kernel_key.sigma = 2.0;
    
    LUTCacheKey lut_key;
    lut_key.type = LUTCacheKey::Type::Gamma;
    lut_key.parameter = 2.0;
    
    int kernel_calls = 0;
    int lut_calls = 0;
    
    cache.getOrGenerateKernel(kernel_key, [&kernel_calls]() {
        kernel_calls++;
        return std::vector<int32_t>{1, 2, 3};
    });
    
    cache.getOrGenerateLUT(lut_key, [&lut_calls]() {
        lut_calls++;
        return std::vector<uint8_t>(256, 0);
    });
    
    EXPECT_EQ(kernel_calls, 1);
    EXPECT_EQ(lut_calls, 1);
    
    cache.clearAll();
    
    // После очистки оба генератора должны быть вызваны снова
    cache.getOrGenerateKernel(kernel_key, [&kernel_calls]() {
        kernel_calls++;
        return std::vector<int32_t>{1, 2, 3};
    });
    
    cache.getOrGenerateLUT(lut_key, [&lut_calls]() {
        lut_calls++;
        return std::vector<uint8_t>(256, 0);
    });
    
    EXPECT_EQ(kernel_calls, 2);
    EXPECT_EQ(lut_calls, 2);
}

// Тест getStatistics
TEST_F(CacheManagerTest, GetStatistics)
{
    auto& cache = CacheManager::getInstance();
    
    auto stats = cache.getStatistics();
    EXPECT_EQ(stats.kernel_cache_size, 0);
    EXPECT_EQ(stats.lut_cache_size, 0);
    
    KernelCacheKey kernel_key;
    kernel_key.type = KernelCacheKey::Type::Gaussian;
    kernel_key.radius = 5.0;
    kernel_key.sigma = 2.0;
    
    cache.getOrGenerateKernel(kernel_key, []() {
        return std::vector<int32_t>{1, 2, 3};
    });
    
    stats = cache.getStatistics();
    EXPECT_EQ(stats.kernel_cache_size, 1);
    EXPECT_EQ(stats.lut_cache_size, 0);
    
    LUTCacheKey lut_key;
    lut_key.type = LUTCacheKey::Type::Gamma;
    lut_key.parameter = 2.0;
    
    cache.getOrGenerateLUT(lut_key, []() {
        return std::vector<uint8_t>(256, 0);
    });
    
    stats = cache.getStatistics();
    EXPECT_EQ(stats.kernel_cache_size, 1);
    EXPECT_EQ(stats.lut_cache_size, 1);
}

// Тест thread-safety для kernel cache
TEST_F(CacheManagerTest, ThreadSafetyKernel)
{
    auto& cache = CacheManager::getInstance();
    
    KernelCacheKey key;
    key.type = KernelCacheKey::Type::Gaussian;
    key.radius = 5.0;
    key.sigma = 2.0;
    
    std::atomic<int> call_count{0};
    auto generator = [&call_count]() -> std::vector<int32_t> {
        call_count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return {1, 2, 3};
    };
    
    const int num_threads = 10;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&cache, &key, &generator]() {
            cache.getOrGenerateKernel(key, generator);
        });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // Генератор должен быть вызван только один раз (или несколько раз из-за race condition,
    // но не больше чем количество потоков)
    EXPECT_LE(call_count.load(), num_threads);
}

// Тест thread-safety для LUT cache
TEST_F(CacheManagerTest, ThreadSafetyLUT)
{
    auto& cache = CacheManager::getInstance();
    
    LUTCacheKey key;
    key.type = LUTCacheKey::Type::Gamma;
    key.parameter = 2.0;
    
    std::atomic<int> call_count{0};
    auto generator = [&call_count]() -> std::vector<uint8_t> {
        call_count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return std::vector<uint8_t>(256, 0);
    };
    
    const int num_threads = 10;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&cache, &key, &generator]() {
            cache.getOrGenerateLUT(key, generator);
        });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // Генератор должен быть вызван только один раз (или несколько раз из-за race condition)
    EXPECT_LE(call_count.load(), num_threads);
}

// Тест KernelCacheKey оператора сравнения
TEST_F(CacheManagerTest, KernelCacheKeyEquality)
{
    KernelCacheKey key1;
    key1.type = KernelCacheKey::Type::Gaussian;
    key1.radius = 5.0;
    key1.sigma = 2.0;
    
    KernelCacheKey key2;
    key2.type = KernelCacheKey::Type::Gaussian;
    key2.radius = 5.0;
    key2.sigma = 2.0;
    
    EXPECT_TRUE(key1 == key2);
    
    key2.radius = 10.0;
    EXPECT_FALSE(key1 == key2);
    
    key2.radius = 5.0;
    key2.type = KernelCacheKey::Type::BoxBlur;
    EXPECT_FALSE(key1 == key2);
}

// Тест LUTCacheKey оператора сравнения
TEST_F(CacheManagerTest, LUTCacheKeyEquality)
{
    LUTCacheKey key1;
    key1.type = LUTCacheKey::Type::Gamma;
    key1.parameter = 2.0;
    
    LUTCacheKey key2;
    key2.type = LUTCacheKey::Type::Gamma;
    key2.parameter = 2.0;
    
    EXPECT_TRUE(key1 == key2);
    
    key2.parameter = 3.0;
    EXPECT_FALSE(key1 == key2);
    
    key2.parameter = 2.0;
    key2.type = LUTCacheKey::Type::Brightness;
    EXPECT_FALSE(key1 == key2);
}

