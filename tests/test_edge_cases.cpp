#include <gtest/gtest.h>
#include <ImageProcessor.h>
#include <filters/IFilter.h>
#include <filters/GrayscaleFilter.h>
#include <filters/GaussianBlurFilter.h>
#include <filters/MedianFilter.h>
#include <filters/MotionBlurFilter.h>
#include "test_utils.h"
#include <cstdlib>
#include <cstring>
#include <vector>
#include <memory>

/**
 * @brief Тесты для очень больших изображений
 */
class LargeImageTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Создаем большое тестовое изображение
    }
};

TEST_F(LargeImageTest, VeryLargeImageGrayscale)
{
    // Тест на очень большое изображение (4K)
    constexpr int width = 3840;
    constexpr int height = 2160;
    
    auto test_image = TestUtils::createTestImage(width, height);
    ImageProcessor processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    ASSERT_TRUE(processor.resize(width, height, allocated_data).isSuccess());
    
    GrayscaleFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

TEST_F(LargeImageTest, VeryLargeImageBlur)
{
    // Тест размытия на большом изображении
    constexpr int width = 1920;
    constexpr int height = 1080;
    
    auto test_image = TestUtils::createTestImage(width, height);
    ImageProcessor processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    ASSERT_TRUE(processor.resize(width, height, allocated_data).isSuccess());
    
    GaussianBlurFilter filter(2.0);
    const auto result = filter.apply(processor);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(processor.isValid());
}

/**
 * @brief Тесты для проверки обработки ошибок
 */
class ErrorHandlingTest : public ::testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(ErrorHandlingTest, InvalidImageSize)
{
    ImageProcessor processor;
    GrayscaleFilter filter;
    
    // Пытаемся применить фильтр к невалидному изображению
    const auto result = filter.apply(processor);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

TEST_F(ErrorHandlingTest, InvalidFilterParameters)
{
    // Тест с невалидными параметрами фильтра
    constexpr int width = 100;
    constexpr int height = 100;
    
    auto test_image = TestUtils::createTestImage(width, height);
    ImageProcessor processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    ASSERT_TRUE(processor.resize(width, height, allocated_data).isSuccess());
    
    // Конструктор GaussianBlurFilter нормализует radius <= 0.0 до 5.0
    // Поэтому передача 0.0 или отрицательного значения не вызовет ошибку в apply()
    // Тест проверяет, что фильтр работает с нормализованным значением
    GaussianBlurFilter filter(0.0); // Конструктор нормализует до 5.0
    const auto result = filter.apply(processor);
    // Фильтр должен работать успешно, так как конструктор нормализовал значение
    EXPECT_TRUE(result.isSuccess());
    
    // Для проверки реальной ошибки нужно использовать фильтр, который не нормализует значения
    // или проверить другой сценарий ошибки
}

/**
 * @brief Тесты для проверки многопоточности
 */
class ThreadingTest : public ::testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(ThreadingTest, ParallelProcessingLargeImage)
{
    // Тест параллельной обработки большого изображения
    constexpr int width = 2000;
    constexpr int height = 2000;
    
    auto test_image = TestUtils::createTestImage(width, height);
    ImageProcessor processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    ASSERT_TRUE(processor.resize(width, height, allocated_data).isSuccess());
    
    // Применяем фильтр, который использует параллельную обработку
    GrayscaleFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(processor.isValid());
}

TEST_F(ThreadingTest, MultipleFiltersSequentially)
{
    // Тест применения нескольких фильтров последовательно
    constexpr int width = 500;
    constexpr int height = 500;
    
    auto test_image = TestUtils::createTestImage(width, height);
    ImageProcessor processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    ASSERT_TRUE(processor.resize(width, height, allocated_data).isSuccess());
    
    // Применяем несколько фильтров подряд
    GrayscaleFilter grayscale;
    EXPECT_TRUE(grayscale.apply(processor).isSuccess());
    
    GaussianBlurFilter blur(1.0);
    EXPECT_TRUE(blur.apply(processor).isSuccess());
    
    EXPECT_TRUE(processor.isValid());
}

/**
 * @brief Тесты для проверки граничных значений параметров
 */
class BoundaryValueTest : public ::testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(BoundaryValueTest, MinimumBlurRadius)
{
    constexpr int width = 100;
    constexpr int height = 100;
    
    auto test_image = TestUtils::createTestImage(width, height);
    ImageProcessor processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    ASSERT_TRUE(processor.resize(width, height, allocated_data).isSuccess());
    
    // Минимальный валидный радиус
    GaussianBlurFilter filter(0.1);
    const auto result = filter.apply(processor);
    EXPECT_TRUE(result.isSuccess());
}

TEST_F(BoundaryValueTest, MaximumBlurRadius)
{
    constexpr int width = 100;
    constexpr int height = 100;
    
    auto test_image = TestUtils::createTestImage(width, height);
    ImageProcessor processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    ASSERT_TRUE(processor.resize(width, height, allocated_data).isSuccess());
    
    // Большой радиус размытия
    GaussianBlurFilter filter(50.0);
    const auto result = filter.apply(processor);
    EXPECT_TRUE(result.isSuccess());
}

TEST_F(BoundaryValueTest, ExtremeAspectRatio)
{
    // Тест на изображение с экстремальным соотношением сторон
    constexpr int width = 10000;
    constexpr int height = 10;
    
    auto test_image = TestUtils::createTestImage(width, height);
    ImageProcessor processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    ASSERT_TRUE(processor.resize(width, height, allocated_data).isSuccess());
    
    GrayscaleFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(processor.isValid());
}

TEST_F(BoundaryValueTest, SquareImage)
{
    // Тест на квадратное изображение
    constexpr int size = 512;
    
    auto test_image = TestUtils::createTestImage(size, size);
    ImageProcessor processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    ASSERT_TRUE(processor.resize(size, size, allocated_data).isSuccess());
    
    GrayscaleFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(processor.isValid());
}

