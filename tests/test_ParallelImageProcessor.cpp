#include <gtest/gtest.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <atomic>

/**
 * @brief Тесты для ParallelImageProcessor
 */
class ParallelImageProcessorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    /**
     * @brief Создает ImageProcessor с тестовым изображением
     */
    ImageProcessor createImageProcessor(int width, int height)
    {
        std::vector<uint8_t> image(static_cast<size_t>(width) * height * 3);
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const auto index = static_cast<size_t>(y * width + x) * 3;
                image[index + 0] = static_cast<uint8_t>(x % 256); // R
                image[index + 1] = static_cast<uint8_t>(y % 256); // G
                image[index + 2] = static_cast<uint8_t>((x + y) % 256); // B
            }
        }
        
        ImageProcessor processor;
        auto* allocated_data = static_cast<uint8_t*>(std::malloc(image.size()));
        if (allocated_data == nullptr)
        {
            ADD_FAILURE() << "Failed to allocate memory";
            return processor;
        }
        std::memcpy(allocated_data, image.data(), image.size());
        
        if (!processor.resize(width, height, allocated_data))
        {
            ADD_FAILURE() << "Failed to resize processor";
            std::free(allocated_data);
            return processor;
        }
        return processor;
    }
};

// Тест getOptimalThreadCount
TEST_F(ParallelImageProcessorTest, GetOptimalThreadCount)
{
    const auto thread_count = ParallelImageProcessor::getOptimalThreadCount();
    EXPECT_GT(thread_count, 0);
    // Обычно количество потоков должно быть равно количеству ядер или больше 1
    EXPECT_LE(thread_count, 128); // Разумный максимум
}

// Тест processRowsParallel с одним потоком
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelSingleThread)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    std::atomic<int> processed_rows{0};
    
    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [&processed_rows](int start_row, int end_row)
        {
            processed_rows += (end_row - start_row);
        },
        nullptr,
        1 // Один поток
    );
    
    EXPECT_EQ(processed_rows.load(), height);
}

// Тест processRowsParallel с несколькими потоками
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelMultipleThreads)
{
    constexpr int width = 100;
    constexpr int height = 100;
    auto processor = createImageProcessor(width, height);
    
    std::atomic<int> processed_rows{0};
    std::vector<std::atomic<bool>> row_processed(static_cast<size_t>(height));
    for (auto& flag : row_processed)
    {
        flag = false;
    }
    
    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [&processed_rows, &row_processed](int start_row, int end_row)
        {
            for (int row = start_row; row < end_row; ++row)
            {
                row_processed[static_cast<size_t>(row)] = true;
                processed_rows++;
            }
        },
        nullptr,
        4 // 4 потока
    );
    
    EXPECT_EQ(processed_rows.load(), height);
    
    // Проверяем, что все строки обработаны
    for (int i = 0; i < height; ++i)
    {
        EXPECT_TRUE(row_processed[static_cast<size_t>(i)].load());
    }
}

// Тест processRowsParallel с нулевой высотой
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelZeroHeight)
{
    bool called = false;
    
    ParallelImageProcessor::processRowsParallel(
        0,
        [&called](int start_row, int end_row)
        {
            called = true;
        },
        nullptr,
        0
    );
    
    // Функция не должна быть вызвана при нулевой высоте
    EXPECT_FALSE(called);
}

// Тест processRowsParallel с отрицательной высотой
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelNegativeHeight)
{
    bool called = false;
    
    ParallelImageProcessor::processRowsParallel(
        -10,
        [&called](int start_row, int end_row)
        {
            called = true;
        },
        nullptr,
        0
    );
    
    // Функция не должна быть вызвана при отрицательной высоте
    EXPECT_FALSE(called);
}

// Тест processRowsParallel с автоматическим определением потоков
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelAutoThreads)
{
    constexpr int width = 50;
    constexpr int height = 50;
    auto processor = createImageProcessor(width, height);
    
    std::atomic<int> processed_rows{0};
    
    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [&processed_rows](int start_row, int end_row)
        {
            processed_rows += (end_row - start_row);
        },
        nullptr,
        0 // Автоматическое определение
    );
    
    EXPECT_EQ(processed_rows.load(), height);
}

// Тест processRowsParallel с маленьким изображением
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelSmallImage)
{
    constexpr int width = 2;
    constexpr int height = 2;
    auto processor = createImageProcessor(width, height);
    
    std::atomic<int> processed_rows{0};
    
    // Даже при запросе многих потоков, для маленького изображения должен использоваться один поток
    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [&processed_rows](int start_row, int end_row)
        {
            processed_rows += (end_row - start_row);
        },
        nullptr,
        10 // Запрашиваем 10 потоков, но для height=2 должен использоваться 1
    );
    
    EXPECT_EQ(processed_rows.load(), height);
}

// Тест корректности диапазонов строк
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelCorrectRanges)
{
    constexpr int height = 100;
    
    std::vector<bool> row_processed(static_cast<size_t>(height), false);
    
    constexpr int width = 100;
    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [&row_processed, height](int start_row, int end_row)
        {
            // Проверяем, что диапазон корректен
            EXPECT_GE(start_row, 0);
            EXPECT_LE(start_row, end_row);
            EXPECT_LE(end_row, height);
            
            // Отмечаем обработанные строки
            for (int row = start_row; row < end_row; ++row)
            {
                EXPECT_FALSE(row_processed[static_cast<size_t>(row)]) 
                    << "Строка " << row << " обработана дважды";
                row_processed[static_cast<size_t>(row)] = true;
            }
        },
        nullptr,
        4
    );
    
    // Проверяем, что все строки обработаны ровно один раз
    for (int i = 0; i < height; ++i)
    {
        EXPECT_TRUE(row_processed[static_cast<size_t>(i)]) 
            << "Строка " << i << " не обработана";
    }
}
