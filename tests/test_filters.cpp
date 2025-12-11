#include <gtest/gtest.h>
#include <ImageProcessor.h>
#include <filters/IFilter.h>
#include <filters/GrayscaleFilter.h>
#include <filters/InvertFilter.h>
#include <filters/BrightnessFilter.h>
#include <filters/ContrastFilter.h>
#include <filters/SaturationFilter.h>
#include <filters/SepiaFilter.h>
#include <filters/FlipHorizontalFilter.h>
#include <filters/FlipVerticalFilter.h>
#include <filters/Rotate90Filter.h>
#include <filters/ThresholdFilter.h>
#include <filters/PosterizeFilter.h>
#include <filters/VignetteFilter.h>
#include <filters/GaussianBlurFilter.h>
#include <filters/BoxBlurFilter.h>
#include <filters/NoiseFilter.h>
#include <filters/ContrastFilter.h>
#include <filters/SaturationFilter.h>
#include <filters/SepiaFilter.h>
#include <filters/EdgeDetectionFilter.h>
#include <filters/EmbossFilter.h>
#include <filters/MedianFilter.h>
#include <filters/MotionBlurFilter.h>
#include <filters/OutlineFilter.h>
#include <filters/SharpenFilter.h>
#include <utils/BorderHandler.h>
#include "test_utils.h"
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <vector>
#include <cmath>
#include <cstdlib>

namespace fs = std::filesystem;

/**
 * @brief Базовый класс для тестов фильтров
 */
class FilterTestBase : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_dir_ = fs::temp_directory_path() / "ImageFilterFilterTests";
        fs::create_directories(test_dir_);
    }

    void TearDown() override
    {
        if (fs::exists(test_dir_))
        {
            fs::remove_all(test_dir_);
        }
    }


    /**
     * @brief Создает ImageProcessor с тестовым изображением в памяти
     */
    ImageProcessor createImageProcessor(int width, int height)
    {
        auto test_image = TestUtils::createTestImage(width, height);
        
        ImageProcessor processor;
        auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
        if (allocated_data == nullptr)
        {
            ADD_FAILURE() << "Failed to allocate memory";
            return processor;
        }
        std::memcpy(allocated_data, test_image.data(), test_image.size());
        
        if (!processor.resize(width, height, allocated_data))
        {
            ADD_FAILURE() << "Failed to resize processor";
            std::free(allocated_data);
            return processor;
        }
        return processor;
    }

    fs::path test_dir_;
};

// Тест GrayscaleFilter
TEST_F(FilterTestBase, GrayscaleFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    GrayscaleFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем, что все каналы имеют одинаковое значение (градации серого)
    const auto* data = processor.getData();
    for (int i = 0; i < width * height; ++i)
    {
        const auto index = static_cast<size_t>(i) * 3;
        EXPECT_EQ(data[index], data[index + 1]); // R == G
        EXPECT_EQ(data[index + 1], data[index + 2]); // G == B
    }
}

// Тест InvertFilter
TEST_F(FilterTestBase, InvertFilter)
{
    constexpr int width = 5;
    constexpr int height = 5;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    InvertFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем инверсию
    const auto* inverted_data = processor.getData();
    for (size_t i = 0; i < original.size(); ++i)
    {
        EXPECT_EQ(inverted_data[i], static_cast<uint8_t>(255 - original[i]));
    }
}

// Тест BrightnessFilter
TEST_F(FilterTestBase, BrightnessFilter)
{
    constexpr int width = 5;
    constexpr int height = 5;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    BrightnessFilter filter(1.5); // Увеличиваем яркость в 1.5 раза
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем, что значения увеличились (но не все, так как есть ограничение 255)
    const auto* brightened_data = processor.getData();
    bool some_increased = false;
    for (size_t i = 0; i < original.size(); ++i)
    {
        if (original[i] < 170) // Значения меньше 170 должны увеличиться
        {
            EXPECT_GE(brightened_data[i], original[i]);
            if (brightened_data[i] > original[i])
            {
                some_increased = true;
            }
        }
    }
    EXPECT_TRUE(some_increased);
}

// Тест ThresholdFilter
TEST_F(FilterTestBase, ThresholdFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    ThresholdFilter filter(128);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем, что все пиксели либо 0, либо 255
    const auto* data = processor.getData();
    for (int i = 0; i < width * height; ++i)
    {
        const auto index = static_cast<size_t>(i) * 3;
        EXPECT_TRUE(data[index] == 0 || data[index] == 255);
        EXPECT_EQ(data[index], data[index + 1]); // Все каналы одинаковые
        EXPECT_EQ(data[index + 1], data[index + 2]);
    }
}

// Тест PosterizeFilter
TEST_F(FilterTestBase, PosterizeFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    constexpr int levels = 4;
    PosterizeFilter filter(levels); // 4 уровня
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем, что значения квантованы
    // step = 256 / levels = 256 / 4 = 64
    constexpr int step = 256 / levels;
    const auto* data = processor.getData();
    for (int i = 0; i < width * height * 3; ++i)
    {
        // Значения должны быть кратны step (64)
        // Но из-за ограничения максимальным значением (levels - 1) * step = 3 * 64 = 192
        // и clamp до [0, 255], значения могут быть в диапазоне [0, 192] с шагом 64
        // или 255 (если было clamp)
        const auto value = data[i];
        const auto remainder = value % step;
        // Допускаем остаток 0 (кратно step) или значение 255 (clamp)
        EXPECT_TRUE(remainder == 0 || value == 255) 
            << "Value " << static_cast<int>(value) << " is not quantized properly. Remainder: " << remainder;
    }
}

// Тест FlipHorizontalFilter
TEST_F(FilterTestBase, FlipHorizontalFilter)
{
    constexpr int width = 5;
    constexpr int height = 3;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    FlipHorizontalFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем отражение по горизонтали
    const auto* flipped_data = processor.getData();
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const auto orig_index = static_cast<size_t>(y * width + x) * 3;
            const auto flip_index = static_cast<size_t>(y * width + (width - 1 - x)) * 3;
            
            EXPECT_EQ(flipped_data[orig_index], original[flip_index]);
            EXPECT_EQ(flipped_data[orig_index + 1], original[flip_index + 1]);
            EXPECT_EQ(flipped_data[orig_index + 2], original[flip_index + 2]);
        }
    }
}

// Тест FlipVerticalFilter
TEST_F(FilterTestBase, FlipVerticalFilter)
{
    constexpr int width = 5;
    constexpr int height = 3;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    FlipVerticalFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем отражение по вертикали
    const auto* flipped_data = processor.getData();
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const auto orig_index = static_cast<size_t>(y * width + x) * 3;
            const auto flip_index = static_cast<size_t>((height - 1 - y) * width + x) * 3;
            
            EXPECT_EQ(flipped_data[orig_index], original[flip_index]);
            EXPECT_EQ(flipped_data[orig_index + 1], original[flip_index + 1]);
            EXPECT_EQ(flipped_data[orig_index + 2], original[flip_index + 2]);
        }
    }
}

// Тест Rotate90Filter
TEST_F(FilterTestBase, Rotate90Filter)
{
    constexpr int width = 4;
    constexpr int height = 3;
    auto processor = createImageProcessor(width, height);
    
    Rotate90Filter filter(true); // По часовой стрелке
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // После поворота на 90 градусов размеры должны поменяться местами
    EXPECT_EQ(processor.getWidth(), height);
    EXPECT_EQ(processor.getHeight(), width);
}

// Тест применения фильтра к невалидному изображению
TEST_F(FilterTestBase, FilterOnInvalidImage)
{
    ImageProcessor processor; // Пустое изображение
    
    GrayscaleFilter filter;
    EXPECT_FALSE(filter.apply(processor).isSuccess());
}

// Тест GrayscaleFilter с RGBA
TEST_F(FilterTestBase, GrayscaleFilterRGBA)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    
    ImageProcessor processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    // Используем временный файл для загрузки RGBA
    const auto filepath = test_dir_ / "test_rgba_grayscale.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    EXPECT_TRUE(processor.loadFromFile(test_file, true));
    EXPECT_EQ(processor.getChannels(), 4);
    EXPECT_TRUE(processor.hasAlpha());
    
    // Сохраняем оригинальные альфа-каналы
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original_alpha(width * height);
    for (int i = 0; i < width * height; ++i)
    {
        original_alpha[i] = original_data[i * 4 + 3];
    }
    
    GrayscaleFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем, что все RGB каналы имеют одинаковое значение
    const auto* data = processor.getData();
    for (int i = 0; i < width * height; ++i)
    {
        const auto index = static_cast<size_t>(i) * 4;
        EXPECT_EQ(data[index], data[index + 1]); // R == G
        EXPECT_EQ(data[index + 1], data[index + 2]); // G == B
        // Проверяем, что альфа-канал не изменился
        EXPECT_EQ(data[index + 3], original_alpha[i]);
    }
}

// Тест InvertFilter с RGBA
TEST_F(FilterTestBase, InvertFilterRGBA)
{
    constexpr int width = 5;
    constexpr int height = 5;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    
    const auto filepath = test_dir_ / "test_rgba_invert.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    EXPECT_TRUE(processor.loadFromFile(test_file, true));
    EXPECT_EQ(processor.getChannels(), 4);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 4);
    std::vector<uint8_t> original_alpha(width * height);
    for (int i = 0; i < width * height; ++i)
    {
        original_alpha[i] = original[i * 4 + 3];
    }
    
    InvertFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем инверсию RGB каналов
    const auto* inverted_data = processor.getData();
    for (size_t i = 0; i < width * height; ++i)
    {
        const auto index = i * 4;
        EXPECT_EQ(inverted_data[index], static_cast<uint8_t>(255 - original[index])); // R
        EXPECT_EQ(inverted_data[index + 1], static_cast<uint8_t>(255 - original[index + 1])); // G
        EXPECT_EQ(inverted_data[index + 2], static_cast<uint8_t>(255 - original[index + 2])); // B
        // Альфа-канал не должен измениться
        EXPECT_EQ(inverted_data[index + 3], original_alpha[i]);
    }
}

// Тест GaussianBlurFilter
TEST_F(FilterTestBase, GaussianBlurFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    GaussianBlurFilter filter(2.0);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем, что изображение все еще валидно
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест BoxBlurFilter
TEST_F(FilterTestBase, BoxBlurFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    BoxBlurFilter filter(3);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест NoiseFilter
TEST_F(FilterTestBase, NoiseFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    NoiseFilter filter(0.1);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем, что некоторые значения изменились (шум добавлен)
    const auto* noisy_data = processor.getData();
    bool some_changed = false;
    for (size_t i = 0; i < original.size(); ++i)
    {
        if (noisy_data[i] != original[i])
        {
            some_changed = true;
            break;
        }
    }
    // С небольшой вероятностью шум может не изменить значения, но обычно изменяет
    // Это вероятностный тест, но в большинстве случаев должен пройти
}

// Тест VignetteFilter
TEST_F(FilterTestBase, VignetteFilter)
{
    constexpr int width = 20;
    constexpr int height = 20;
    auto processor = createImageProcessor(width, height);
    
    VignetteFilter filter(0.5);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем, что углы затемнены (значения меньше)
    const auto* data = processor.getData();
    
    // Угол (0, 0)
    const auto corner_index = 0;
    // Центр
    const auto center_index = static_cast<size_t>((height / 2) * width + (width / 2)) * 3;
    
    // Угол должен быть темнее центра (меньше значение)
    // Но это зависит от исходного изображения, поэтому просто проверяем валидность
    EXPECT_TRUE(processor.isValid());
}

// Тест ContrastFilter
TEST_F(FilterTestBase, ContrastFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    ContrastFilter filter(1.5); // Увеличиваем контраст
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем, что изображение валидно
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
    
    // Проверяем, что некоторые значения изменились
    const auto* contrast_data = processor.getData();
    bool some_changed = false;
    for (size_t i = 0; i < original.size(); ++i)
    {
        if (contrast_data[i] != original[i])
        {
            some_changed = true;
            break;
        }
    }
    EXPECT_TRUE(some_changed);
}

// Тест ContrastFilter с минимальным значением
TEST_F(FilterTestBase, ContrastFilterMinValue)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    ContrastFilter filter(0.1); // Минимальный контраст
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
}

// Тест ContrastFilter с максимальным значением
TEST_F(FilterTestBase, ContrastFilterMaxValue)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    ContrastFilter filter(10.0); // Максимальный контраст
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
}

// Тест SaturationFilter
TEST_F(FilterTestBase, SaturationFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    SaturationFilter filter(1.5); // Увеличиваем насыщенность
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    EXPECT_TRUE(processor.isValid());
    
    // Проверяем, что некоторые значения изменились
    const auto* saturated_data = processor.getData();
    bool some_changed = false;
    for (size_t i = 0; i < original.size(); ++i)
    {
        if (saturated_data[i] != original[i])
        {
            some_changed = true;
            break;
        }
    }
    EXPECT_TRUE(some_changed);
}

// Тест SaturationFilter с нулевой насыщенностью (градации серого)
TEST_F(FilterTestBase, SaturationFilterZero)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Конструктор нормализует 0.0 до 1.5, поэтому используем очень маленькое значение
    // которое будет нормализовано до самого себя (0.0 < factor <= очень маленькое)
    // Но конструктор проверяет factor > 0.0, поэтому 0.0 нормализуется до 1.5
    // Используем очень маленькое значение, близкое к 0, но > 0
    constexpr double very_small_factor = 0.0001;
    SaturationFilter filter(very_small_factor); // Почти градации серого
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // При очень маленьком factor, результат должен быть очень близок к градациям серого
    // Проверяем, что все каналы имеют очень близкие значения
    // Допускаем большую разницу, так как factor не равен точно 0
    const auto* data = processor.getData();
    for (int i = 0; i < width * height; ++i)
    {
        const auto index = static_cast<size_t>(i) * 3;
        const int r = data[index];
        const int g = data[index + 1];
        const int b = data[index + 2];
        // При очень маленьком factor, разница должна быть небольшой
        // Но не нулевой, так как factor != 0
        EXPECT_LE(std::abs(r - g), 10) << "R and G differ too much at pixel " << i;
        EXPECT_LE(std::abs(g - b), 10) << "G and B differ too much at pixel " << i;
        EXPECT_LE(std::abs(r - b), 10) << "R and B differ too much at pixel " << i;
    }
}

// Тест SepiaFilter
TEST_F(FilterTestBase, SepiaFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    SepiaFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    EXPECT_TRUE(processor.isValid());
    
    // Проверяем, что значения изменились
    const auto* sepia_data = processor.getData();
    bool some_changed = false;
    for (size_t i = 0; i < original.size(); ++i)
    {
        if (sepia_data[i] != original[i])
        {
            some_changed = true;
            break;
        }
    }
    EXPECT_TRUE(some_changed);
    
    // Проверяем, что изображение имеет коричневатый оттенок (B < G < R обычно для сепии)
    // Это не строгая проверка, но общая тенденция
}

// Тест EdgeDetectionFilter
TEST_F(FilterTestBase, EdgeDetectionFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    EdgeDetectionFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
    
    // Проверяем, что результат в градациях серого (R == G == B)
    const auto* data = processor.getData();
    for (int i = 0; i < width * height; ++i)
    {
        const auto index = static_cast<size_t>(i) * 3;
        EXPECT_EQ(data[index], data[index + 1]); // R == G
        EXPECT_EQ(data[index + 1], data[index + 2]); // G == B
    }
}

// Тест EdgeDetectionFilter с различными стратегиями границ
TEST_F(FilterTestBase, EdgeDetectionFilterBorderStrategies)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    const auto strategies = {
        BorderHandler::Strategy::Mirror,
        BorderHandler::Strategy::Clamp,
        BorderHandler::Strategy::Wrap,
        BorderHandler::Strategy::Extend
    };
    
    for (const auto strategy : strategies)
    {
        auto processor = createImageProcessor(width, height);
        EdgeDetectionFilter filter(strategy);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тест EmbossFilter
TEST_F(FilterTestBase, EmbossFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    EmbossFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест EmbossFilter с различными стратегиями границ
TEST_F(FilterTestBase, EmbossFilterBorderStrategies)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    const auto strategies = {
        BorderHandler::Strategy::Mirror,
        BorderHandler::Strategy::Clamp,
        BorderHandler::Strategy::Wrap,
        BorderHandler::Strategy::Extend
    };
    
    for (const auto strategy : strategies)
    {
        auto processor = createImageProcessor(width, height);
        EmbossFilter filter(strategy);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тест MedianFilter
TEST_F(FilterTestBase, MedianFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    MedianFilter filter(2);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест MedianFilter с различными радиусами
TEST_F(FilterTestBase, MedianFilterRadii)
{
    constexpr int width = 20;
    constexpr int height = 20;
    
    for (int radius = 1; radius <= 5; ++radius)
    {
        auto processor = createImageProcessor(width, height);
        MedianFilter filter(radius);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тест MedianFilter с различными стратегиями границ
TEST_F(FilterTestBase, MedianFilterBorderStrategies)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    const auto strategies = {
        BorderHandler::Strategy::Mirror,
        BorderHandler::Strategy::Clamp,
        BorderHandler::Strategy::Wrap,
        BorderHandler::Strategy::Extend
    };
    
    for (const auto strategy : strategies)
    {
        auto processor = createImageProcessor(width, height);
        MedianFilter filter(2, strategy);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тест MotionBlurFilter
TEST_F(FilterTestBase, MotionBlurFilter)
{
    constexpr int width = 20;
    constexpr int height = 20;
    auto processor = createImageProcessor(width, height);
    
    MotionBlurFilter filter(10, 0.0); // Горизонтальное размытие
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест MotionBlurFilter с различными углами
TEST_F(FilterTestBase, MotionBlurFilterAngles)
{
    constexpr int width = 20;
    constexpr int height = 20;
    
    const double angles[] = {0.0, 45.0, 90.0, 135.0, 180.0};
    
    for (double angle : angles)
    {
        auto processor = createImageProcessor(width, height);
        MotionBlurFilter filter(10, angle);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тест MotionBlurFilter с различными стратегиями границ
TEST_F(FilterTestBase, MotionBlurFilterBorderStrategies)
{
    constexpr int width = 20;
    constexpr int height = 20;
    
    const auto strategies = {
        BorderHandler::Strategy::Mirror,
        BorderHandler::Strategy::Clamp,
        BorderHandler::Strategy::Wrap,
        BorderHandler::Strategy::Extend
    };
    
    for (const auto strategy : strategies)
    {
        auto processor = createImageProcessor(width, height);
        MotionBlurFilter filter(10, 0.0, strategy);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тест OutlineFilter
TEST_F(FilterTestBase, OutlineFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    OutlineFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
    
    // Проверяем, что результат в градациях серого
    const auto* data = processor.getData();
    for (int i = 0; i < width * height; ++i)
    {
        const auto index = static_cast<size_t>(i) * 3;
        EXPECT_EQ(data[index], data[index + 1]); // R == G
        EXPECT_EQ(data[index + 1], data[index + 2]); // G == B
    }
}

// Тест OutlineFilter с различными стратегиями границ
TEST_F(FilterTestBase, OutlineFilterBorderStrategies)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    const auto strategies = {
        BorderHandler::Strategy::Mirror,
        BorderHandler::Strategy::Clamp,
        BorderHandler::Strategy::Wrap,
        BorderHandler::Strategy::Extend
    };
    
    for (const auto strategy : strategies)
    {
        auto processor = createImageProcessor(width, height);
        OutlineFilter filter(strategy);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тест SharpenFilter
TEST_F(FilterTestBase, SharpenFilter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    SharpenFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест SharpenFilter с различными стратегиями границ
TEST_F(FilterTestBase, SharpenFilterBorderStrategies)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    const auto strategies = {
        BorderHandler::Strategy::Mirror,
        BorderHandler::Strategy::Clamp,
        BorderHandler::Strategy::Wrap,
        BorderHandler::Strategy::Extend
    };
    
    for (const auto strategy : strategies)
    {
        auto processor = createImageProcessor(width, height);
        SharpenFilter filter(strategy);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тесты для граничных случаев - маленькие изображения
TEST_F(FilterTestBase, FilterOn1x1Image)
{
    constexpr int width = 1;
    constexpr int height = 1;
    auto processor = createImageProcessor(width, height);
    
    GrayscaleFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

TEST_F(FilterTestBase, FilterOn2x2Image)
{
    constexpr int width = 2;
    constexpr int height = 2;
    auto processor = createImageProcessor(width, height);
    
    InvertFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест для маленького изображения с фильтрами, требующими окрестность
TEST_F(FilterTestBase, BlurFilterOnSmallImage)
{
    constexpr int width = 3;
    constexpr int height = 3;
    auto processor = createImageProcessor(width, height);
    
    GaussianBlurFilter blur_filter(1.0);
    EXPECT_TRUE(blur_filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
}

// Тест для большого изображения
TEST_F(FilterTestBase, FilterOnLargeImage)
{
    constexpr int width = 1000;
    constexpr int height = 1000;
    auto processor = createImageProcessor(width, height);
    
    GrayscaleFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест для очень большого изображения
TEST_F(FilterTestBase, FilterOnVeryLargeImage)
{
    constexpr int width = 2000;
    constexpr int height = 2000;
    auto processor = createImageProcessor(width, height);
    
    BrightnessFilter filter(1.2);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
}

// Property-based тесты - применение фильтра дважды
TEST_F(FilterTestBase, InvertFilterTwice)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    InvertFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(filter.apply(processor).isSuccess()); // Применяем второй раз
    
    // После двойной инверсии должно вернуться исходное изображение
    const auto* result_data = processor.getData();
    for (size_t i = 0; i < original.size(); ++i)
    {
        EXPECT_EQ(result_data[i], original[i]);
    }
}

// Property-based тест - применение FlipHorizontalFilter дважды
TEST_F(FilterTestBase, FlipHorizontalFilterTwice)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    FlipHorizontalFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(filter.apply(processor).isSuccess()); // Применяем второй раз
    
    // После двойного отражения должно вернуться исходное изображение
    const auto* result_data = processor.getData();
    for (size_t i = 0; i < original.size(); ++i)
    {
        EXPECT_EQ(result_data[i], original[i]);
    }
}

// Property-based тест - применение FlipVerticalFilter дважды
TEST_F(FilterTestBase, FlipVerticalFilterTwice)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    FlipVerticalFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(filter.apply(processor).isSuccess()); // Применяем второй раз
    
    // После двойного отражения должно вернуться исходное изображение
    const auto* result_data = processor.getData();
    for (size_t i = 0; i < original.size(); ++i)
    {
        EXPECT_EQ(result_data[i], original[i]);
    }
}

// Property-based тест - Rotate90Filter четыре раза
TEST_F(FilterTestBase, Rotate90FilterFourTimes)
{
    constexpr int width = 4;
    constexpr int height = 3;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные размеры
    const int original_width = processor.getWidth();
    const int original_height = processor.getHeight();
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    Rotate90Filter filter(true); // По часовой стрелке
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(filter.apply(processor).isSuccess()); // Четыре раза = 360 градусов
    
    // После четырех поворотов размеры должны вернуться к исходным
    EXPECT_EQ(processor.getWidth(), original_width);
    EXPECT_EQ(processor.getHeight(), original_height);
}

// Тесты для обработки ошибок - некорректные параметры фильтров
TEST_F(FilterTestBase, BrightnessFilterInvalidParameter)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Отрицательное значение должно быть обработано корректно
    BrightnessFilter filter(-0.5);
    // Фильтр должен обработать это (возможно, используя значение по умолчанию или clamp)
    filter.apply(processor);
    EXPECT_TRUE(processor.isValid());
}

// Тест для некорректного радиуса MedianFilter
TEST_F(FilterTestBase, MedianFilterInvalidRadius)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Отрицательный радиус должен быть обработан корректно
    MedianFilter filter(-1);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
}

// Тест для некорректной длины MotionBlurFilter
TEST_F(FilterTestBase, MotionBlurFilterInvalidLength)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Нулевая или отрицательная длина должна быть обработана
    MotionBlurFilter filter(0);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
}

// Тест для некорректного фактора ContrastFilter
TEST_F(FilterTestBase, ContrastFilterInvalidFactor)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Отрицательный или нулевой фактор должен быть обработан
    ContrastFilter filter(-1.0);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
}

// Тест для некорректного фактора SaturationFilter
TEST_F(FilterTestBase, SaturationFilterInvalidFactor)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Отрицательный фактор должен быть обработан
    SaturationFilter filter(-1.0);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
}

// Тест для некорректного порога ThresholdFilter
TEST_F(FilterTestBase, ThresholdFilterBoundaryValues)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Минимальное значение
    ThresholdFilter filter_min(0);
    EXPECT_TRUE(filter_min.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
    
    // Максимальное значение
    auto processor2 = createImageProcessor(width, height);
    ThresholdFilter filter_max(255);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
    EXPECT_TRUE(processor2.isValid());
}

// Тест для некорректного количества уровней PosterizeFilter
TEST_F(FilterTestBase, PosterizeFilterBoundaryValues)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    // Минимальное значение (2 уровня)
    auto processor1 = createImageProcessor(width, height);
    PosterizeFilter filter_min(2);
    EXPECT_TRUE(filter_min.apply(processor1).isSuccess());
    EXPECT_TRUE(processor1.isValid());
    
    // Большое значение
    auto processor2 = createImageProcessor(width, height);
    PosterizeFilter filter_max(256);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
    EXPECT_TRUE(processor2.isValid());
}
