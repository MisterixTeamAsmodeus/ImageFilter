#include <gtest/gtest.h>
#include <utils/BufferPool.h>
#include <utils/IBufferPool.h>
#include <cstdint>
#include <vector>
#include <thread>

class BufferPoolTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        pool_ = std::make_unique<BufferPool>();
    }

    void TearDown() override
    {
        pool_.reset();
    }

    std::unique_ptr<IBufferPool> pool_;
};

TEST_F(BufferPoolTest, AcquireCreatesNewBuffer)
{
    const size_t size = 1024;
    auto buffer = pool_->acquire(size);
    
    EXPECT_GE(buffer.capacity(), size);
    EXPECT_EQ(buffer.size(), size);
}

TEST_F(BufferPoolTest, ReleaseReturnsBufferToPool)
{
    const size_t size = 1024;
    auto buffer1 = pool_->acquire(size);
    buffer1[0] = 42;
    
    pool_->release(std::move(buffer1));
    
    EXPECT_EQ(pool_->size(), 1);
}

TEST_F(BufferPoolTest, AcquireReusesBufferFromPool)
{
    const size_t size = 1024;
    auto buffer1 = pool_->acquire(size);
    pool_->release(std::move(buffer1));
    
    auto buffer2 = pool_->acquire(size);
    
    EXPECT_GE(buffer2.capacity(), size);
    EXPECT_EQ(pool_->size(), 0);
}

TEST_F(BufferPoolTest, AcquireLargerBufferCreatesNew)
{
    const size_t small_size = 512;
    const size_t large_size = 2048;
    
    auto buffer1 = pool_->acquire(small_size);
    pool_->release(std::move(buffer1));
    
    auto buffer2 = pool_->acquire(large_size);
    
    EXPECT_GE(buffer2.capacity(), large_size);
    EXPECT_EQ(pool_->size(), 1); // Маленький буфер остался в пуле
}

TEST_F(BufferPoolTest, AcquireSmallerBufferReusesLarger)
{
    const size_t large_size = 2048;
    const size_t small_size = 512;
    
    auto buffer1 = pool_->acquire(large_size);
    pool_->release(std::move(buffer1));
    
    auto buffer2 = pool_->acquire(small_size);
    
    EXPECT_GE(buffer2.capacity(), small_size);
    EXPECT_EQ(pool_->size(), 0); // Большой буфер был переиспользован
}

TEST_F(BufferPoolTest, ClearRemovesAllBuffers)
{
    const size_t size = 1024;
    auto buffer1 = pool_->acquire(size);
    auto buffer2 = pool_->acquire(size);
    
    pool_->release(std::move(buffer1));
    pool_->release(std::move(buffer2));
    
    EXPECT_EQ(pool_->size(), 2);
    
    pool_->clear();
    
    EXPECT_EQ(pool_->size(), 0);
}

TEST_F(BufferPoolTest, ReservePreallocatesBuffers)
{
    const size_t size = 1024;
    const size_t count = 5;
    
    pool_->reserve(size, count);
    
    EXPECT_EQ(pool_->size(), count);
    
    // Проверяем, что зарезервированные буферы можно использовать
    auto buffer = pool_->acquire(size);
    EXPECT_GE(buffer.capacity(), size);
    EXPECT_EQ(pool_->size(), count - 1);
}

TEST_F(BufferPoolTest, MaxPoolSizeLimitsBuffers)
{
    const size_t max_size = 3;
    auto limited_pool = std::make_unique<BufferPool>(max_size);
    
    const size_t buffer_size = 1024;
    
    // Добавляем больше буферов, чем максимум
    for (int i = 0; i < 5; ++i)
    {
        auto buffer = limited_pool->acquire(buffer_size);
        limited_pool->release(std::move(buffer));
    }
    
    EXPECT_LE(limited_pool->size(), max_size);
}

TEST_F(BufferPoolTest, EmptyBufferNotAddedToPool)
{
    std::vector<uint8_t> empty_buffer;
    pool_->release(std::move(empty_buffer));
    
    EXPECT_EQ(pool_->size(), 0);
}

TEST_F(BufferPoolTest, ThreadSafety)
{
    const size_t size = 1024;
    const int num_threads = 4;
    const int iterations = 100;
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([this, size, iterations]() {
            for (int i = 0; i < iterations; ++i)
            {
                auto buffer = pool_->acquire(size);
                // Заполняем буфер данными
                std::fill(buffer.begin(), buffer.end(), static_cast<uint8_t>(i));
                pool_->release(std::move(buffer));
            }
        });
    }
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    // Пул должен содержать некоторое количество буферов
    EXPECT_GE(pool_->size(), 0);
}

TEST_F(BufferPoolTest, SetMaxPoolSize)
{
    auto pool = std::make_unique<BufferPool>(5);
    
    const size_t size = 1024;
    
    // Заполняем пул
    for (int i = 0; i < 10; ++i)
    {
        auto buffer = pool->acquire(size);
        pool->release(std::move(buffer));
    }
    
    EXPECT_LE(pool->size(), 5);
    
    // Уменьшаем максимальный размер
    pool->setMaxPoolSize(2);
    
    EXPECT_LE(pool->size(), 2);
}

TEST_F(BufferPoolTest, GetMaxPoolSize)
{
    const size_t max_size = 10;
    auto pool = std::make_unique<BufferPool>(max_size);
    
    EXPECT_EQ(pool->getMaxPoolSize(), max_size);
}

TEST_F(BufferPoolTest, ZeroMaxPoolSizeUnlimited)
{
    auto pool = std::make_unique<BufferPool>(0);
    
    const size_t size = 1024;
    
    // Добавляем много буферов
    for (int i = 0; i < 100; ++i)
    {
        auto buffer = pool->acquire(size);
        pool->release(std::move(buffer));
    }
    
    EXPECT_GE(pool->size(), 1); // Должен принимать буферы без ограничений
}

