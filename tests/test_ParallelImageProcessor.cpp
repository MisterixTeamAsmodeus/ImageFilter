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
        
        const auto resize_result = processor.resize(width, height, allocated_data);
        if (!resize_result.isSuccess())
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
        [&called](int /* start_row */, int /* end_row */)
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
        [&called](int /* start_row */, int /* end_row */)
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
    
    // Используем атомарные флаги для thread-safety
    std::vector<std::atomic<bool>> row_processed(static_cast<size_t>(height));
    for (auto& flag : row_processed)
    {
        flag = false;
    }
    
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
                bool expected = false;
                if (!row_processed[static_cast<size_t>(row)].compare_exchange_strong(expected, true))
                {
                    ADD_FAILURE() << "Строка " << row << " обработана дважды";
                }
            }
        },
        nullptr,
        4
    );
    
    // Проверяем, что все строки обработаны ровно один раз
    for (int i = 0; i < height; ++i)
    {
        EXPECT_TRUE(row_processed[static_cast<size_t>(i)].load()) 
            << "Строка " << i << " не обработана";
    }
}

// Тест shouldUseParallelProcessing для маленьких изображений
TEST_F(ParallelImageProcessorTest, ShouldUseParallelProcessingSmall)
{
    // Изображения меньше 100x100 должны использовать последовательную обработку
    EXPECT_FALSE(ParallelImageProcessor::shouldUseParallelProcessing(50, 50));
    EXPECT_FALSE(ParallelImageProcessor::shouldUseParallelProcessing(99, 99));
    EXPECT_FALSE(ParallelImageProcessor::shouldUseParallelProcessing(10, 10));
    EXPECT_FALSE(ParallelImageProcessor::shouldUseParallelProcessing(1, 1));
}

// Тест shouldUseParallelProcessing для больших изображений
TEST_F(ParallelImageProcessorTest, ShouldUseParallelProcessingLarge)
{
    // Изображения >= 100x100 должны использовать параллельную обработку
    EXPECT_TRUE(ParallelImageProcessor::shouldUseParallelProcessing(100, 100));
    EXPECT_TRUE(ParallelImageProcessor::shouldUseParallelProcessing(200, 200));
    EXPECT_TRUE(ParallelImageProcessor::shouldUseParallelProcessing(1000, 1000));
    EXPECT_TRUE(ParallelImageProcessor::shouldUseParallelProcessing(5000, 5000));
}

// Тест shouldUseParallelProcessing для прямоугольных изображений
TEST_F(ParallelImageProcessorTest, ShouldUseParallelProcessingRectangular)
{
    // Проверяем прямоугольные изображения
    // Порог = 100*100 = 10000, поэтому 10*1000 = 10000 >= 10000 -> true
    EXPECT_TRUE(ParallelImageProcessor::shouldUseParallelProcessing(10, 1000)); // 10*1000 = 10000 >= 10000
    EXPECT_TRUE(ParallelImageProcessor::shouldUseParallelProcessing(100, 1000)); // 100*1000 = 100000 >= 10000
    EXPECT_TRUE(ParallelImageProcessor::shouldUseParallelProcessing(1000, 10)); // 1000*10 = 10000 >= 10000
    // Меньше порога
    EXPECT_FALSE(ParallelImageProcessor::shouldUseParallelProcessing(10, 999)); // 10*999 = 9990 < 10000
}

// Тест getAdaptiveThreadCount для маленьких изображений
TEST_F(ParallelImageProcessorTest, GetAdaptiveThreadCountSmall)
{
    // Для маленьких изображений должен возвращаться 1 поток
    EXPECT_EQ(ParallelImageProcessor::getAdaptiveThreadCount(50, 50, 0), 1);
    EXPECT_EQ(ParallelImageProcessor::getAdaptiveThreadCount(10, 10, 0), 1);
    EXPECT_EQ(ParallelImageProcessor::getAdaptiveThreadCount(1, 1, 0), 1);
}

// Тест getAdaptiveThreadCount для больших изображений
TEST_F(ParallelImageProcessorTest, GetAdaptiveThreadCountLarge)
{
    // Для больших изображений должен возвращаться оптимальное количество потоков
    const int optimal = ParallelImageProcessor::getOptimalThreadCount();
    
    const int adaptive = ParallelImageProcessor::getAdaptiveThreadCount(2000, 2000, 0);
    EXPECT_GT(adaptive, 0);
    EXPECT_LE(adaptive, optimal);
}

// Тест getAdaptiveThreadCount с запрошенным количеством потоков
TEST_F(ParallelImageProcessorTest, GetAdaptiveThreadCountRequested)
{
    // Если запрошено конкретное количество потоков, оно должно учитываться
    const int requested = 4;
    const int adaptive = ParallelImageProcessor::getAdaptiveThreadCount(2000, 2000, requested);
    
    // Для больших изображений должно использоваться запрошенное количество
    // или оптимальное, если запрошенное больше оптимального
    EXPECT_GT(adaptive, 0);
    EXPECT_LE(adaptive, std::max(requested, ParallelImageProcessor::getOptimalThreadCount()));
}

// Тест processRowsParallel с нулевой шириной
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelZeroWidth)
{
    bool called = false;
    
    ParallelImageProcessor::processRowsParallel(
        10,
        0,
        [&called](int /* start_row */, int /* end_row */)
        {
            called = true;
        },
        nullptr,
        0
    );
    
    // Функция не должна быть вызвана при нулевой ширине
    EXPECT_FALSE(called);
}

// Тест processRowsParallel с отрицательной шириной
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelNegativeWidth)
{
    bool called = false;
    
    ParallelImageProcessor::processRowsParallel(
        10,
        -10,
        [&called](int /* start_row */, int /* end_row */)
        {
            called = true;
        },
        nullptr,
        0
    );
    
    // Функция не должна быть вызвана при отрицательной ширине
    EXPECT_FALSE(called);
}

// Тест processRowsParallel с очень большим изображением
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelVeryLargeImage)
{
    constexpr int width = 1000;
    constexpr int height = 1000;
    
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

// Тест processRowsParallel с очень маленьким изображением
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelVerySmallImage)
{
    constexpr int width = 1;
    constexpr int height = 1;
    
    std::atomic<int> processed_rows{0};
    
    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [&processed_rows](int start_row, int end_row)
        {
            processed_rows += (end_row - start_row);
        },
        nullptr,
        10 // Запрашиваем много потоков, но для маленького изображения должен использоваться 1
    );
    
    EXPECT_EQ(processed_rows.load(), height);
}

// Тест processRowsParallel с прямоугольным изображением
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelRectangular)
{
    constexpr int width = 200;
    constexpr int height = 50;
    
    std::atomic<int> processed_rows{0};
    // Используем атомарные флаги для thread-safety
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
                bool expected = false;
                if (!row_processed[static_cast<size_t>(row)].compare_exchange_strong(expected, true))
                {
                    ADD_FAILURE() << "Строка " << row << " обработана дважды";
                }
                processed_rows++;
            }
        },
        nullptr,
        4
    );
    
    EXPECT_EQ(processed_rows.load(), height);
    
    // Проверяем, что все строки обработаны
    for (int i = 0; i < height; ++i)
    {
        EXPECT_TRUE(row_processed[static_cast<size_t>(i)].load())
            << "Строка " << i << " не обработана";
    }
}

// Тест processRowsParallel с большим количеством потоков
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelManyThreads)
{
    constexpr int width = 100;
    constexpr int height = 100;
    
    std::atomic<int> processed_rows{0};
    
    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [&processed_rows](int start_row, int end_row)
        {
            processed_rows += (end_row - start_row);
        },
        nullptr,
        16 // Запрашиваем 16 потоков
    );
    
    EXPECT_EQ(processed_rows.load(), height);
}

// Тест processRowsParallel с одним потоком для большого изображения
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelSingleThreadLarge)
{
    constexpr int width = 200;
    constexpr int height = 200;
    
    std::atomic<int> processed_rows{0};
    
    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [&processed_rows](int start_row, int end_row)
        {
            processed_rows += (end_row - start_row);
        },
        nullptr,
        1 // Принудительно один поток
    );
    
    EXPECT_EQ(processed_rows.load(), height);
}

// Тест processRowsParallel с перегрузкой без width
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelWithoutWidth)
{
    constexpr int height = 100;
    
    std::atomic<int> processed_rows{0};
    
    ParallelImageProcessor::processRowsParallel(
        height,
        [&processed_rows](int start_row, int end_row)
        {
            processed_rows += (end_row - start_row);
        },
        nullptr,
        0
    );
    
    EXPECT_EQ(processed_rows.load(), height);
}

// Тест processRowsParallel с перегрузкой без width для маленького изображения
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelWithoutWidthSmall)
{
    constexpr int height = 5;
    
    std::atomic<int> processed_rows{0};
    
    ParallelImageProcessor::processRowsParallel(
        height,
        [&processed_rows](int start_row, int end_row)
        {
            processed_rows += (end_row - start_row);
        },
        nullptr,
        0
    );
    
    EXPECT_EQ(processed_rows.load(), height);
}

// Тест processRowsParallel с перегрузкой без width для нулевой высоты
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelWithoutWidthZeroHeight)
{
    bool called = false;
    
    ParallelImageProcessor::processRowsParallel(
        0,
        [&called](int /* start_row */, int /* end_row */)
        {
            called = true;
        },
        nullptr,
        0
    );
    
    EXPECT_FALSE(called);
}

// Тест thread-safety: параллельная обработка нескольких изображений
TEST_F(ParallelImageProcessorTest, ThreadSafetyMultipleImages)
{
    constexpr int width = 50;
    constexpr int height = 50;
    constexpr int num_images = 10;
    
    std::vector<std::atomic<int>> processed_counts(num_images);
    for (auto& count : processed_counts)
    {
        count = 0;
    }
    
    // Обрабатываем несколько изображений параллельно
    std::vector<std::thread> threads;
    for (int i = 0; i < num_images; ++i)
    {
        threads.emplace_back([i, &processed_counts]()
        {
            constexpr int local_width = 50;
            constexpr int local_height = 50;
            ParallelImageProcessor::processRowsParallel(
                local_height,
                local_width,
                [i, &processed_counts](int start_row, int end_row)
                {
                    processed_counts[i] += (end_row - start_row);
                },
                nullptr,
                0
            );
        });
    }
    
    // Ждем завершения всех потоков
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    // Проверяем, что все изображения обработаны
    for (int i = 0; i < num_images; ++i)
    {
        EXPECT_EQ(processed_counts[i].load(), height) 
            << "Image " << i << " not fully processed";
    }
}

// Тест обработки ошибок в функции обработки строк
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelWithErrors)
{
    constexpr int width = 100;
    constexpr int height = 100;
    
    std::atomic<int> processed_rows{0};
    std::atomic<int> error_count{0};
    
    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [&processed_rows, &error_count](int start_row, int end_row)
        {
            try
            {
                // Симулируем обработку с возможной ошибкой
                if (start_row == 50) // Симулируем ошибку на определенной строке
                {
                    error_count++;
                    throw std::runtime_error("Simulated error");
                }
                processed_rows += (end_row - start_row);
            }
            catch (...)
            {
                // Обрабатываем ошибку
                error_count++;
            }
        },
        nullptr,
        4
    );
    
    // Проверяем, что обработка продолжилась несмотря на ошибки
    EXPECT_GT(processed_rows.load(), 0);
}

// Тест processRowsParallel с различными размерами изображений
TEST_F(ParallelImageProcessorTest, ProcessRowsParallelVariousSizes)
{
    const std::vector<std::pair<int, int>> sizes = {
        {10, 10},   // Маленькое
        {100, 100}, // Среднее
        {500, 500}, // Большое
        {1000, 100} // Прямоугольное большое
    };
    
    for (const auto& [width, height] : sizes)
    {
        std::atomic<int> processed_rows{0};
        
        ParallelImageProcessor::processRowsParallel(
            height,
            width,
            [&processed_rows](int start_row, int end_row)
            {
                processed_rows += (end_row - start_row);
            },
            nullptr,
            0
        );
        
        EXPECT_EQ(processed_rows.load(), height) 
            << "Failed for size " << width << "x" << height;
    }
}
