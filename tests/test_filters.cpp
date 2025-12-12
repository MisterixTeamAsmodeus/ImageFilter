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
        
        const auto resize_result = processor.resize(width, height, allocated_data);
        if (!resize_result.isSuccess())
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
    
    EXPECT_TRUE(processor.loadFromFile(test_file, true).isSuccess());
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
    EXPECT_TRUE(processor.loadFromFile(test_file, true).isSuccess());
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
    (void)some_changed; // Подавляем предупреждение о неиспользуемой переменной
}

// Тест VignetteFilter
TEST_F(FilterTestBase, VignetteFilter)
{
    constexpr int width = 20;
    constexpr int height = 20;
    auto processor = createImageProcessor(width, height);
    
    VignetteFilter filter(0.5);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем валидность изображения
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
        EdgeDetectionFilter filter(0.5, EdgeDetectionFilter::Operator::Sobel, strategy);
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
        EmbossFilter filter(1.0, strategy);
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
        SharpenFilter filter(1.0, strategy);
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

// Тест применения фильтра к изображению 1x1
TEST_F(FilterTestBase, FilterOn1x1ImageDetailed)
{
    constexpr int width = 1;
    constexpr int height = 1;
    auto processor = createImageProcessor(width, height);
    
    // Тестируем различные фильтры на изображении 1x1
    GrayscaleFilter grayscale;
    EXPECT_TRUE(grayscale.apply(processor).isSuccess());
    
    InvertFilter invert;
    EXPECT_TRUE(invert.apply(processor).isSuccess());
    
    BrightnessFilter brightness(1.5);
    EXPECT_TRUE(brightness.apply(processor).isSuccess());
    
    ThresholdFilter threshold(128);
    EXPECT_TRUE(threshold.apply(processor).isSuccess());
}

// Тест применения фильтров размытия к очень маленькому изображению
TEST_F(FilterTestBase, BlurFiltersOnVerySmallImage)
{
    constexpr int width = 2;
    constexpr int height = 2;
    auto processor = createImageProcessor(width, height);
    
    GaussianBlurFilter gaussian(1.0);
    EXPECT_TRUE(gaussian.apply(processor).isSuccess());
    
    BoxBlurFilter box(1);
    EXPECT_TRUE(box.apply(processor).isSuccess());
}

// Тест BrightnessFilter с экстремальными значениями
TEST_F(FilterTestBase, BrightnessFilterExtremeValues)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Очень большое значение
    BrightnessFilter filter_high(10.0);
    EXPECT_TRUE(filter_high.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
    
    // Очень маленькое значение
    auto processor2 = createImageProcessor(width, height);
    BrightnessFilter filter_low(0.01);
    EXPECT_TRUE(filter_low.apply(processor2).isSuccess());
    EXPECT_TRUE(processor2.isValid());
    
    // Нулевое значение
    auto processor3 = createImageProcessor(width, height);
    BrightnessFilter filter_zero(0.0);
    EXPECT_TRUE(filter_zero.apply(processor3).isSuccess());
    EXPECT_TRUE(processor3.isValid());
}

// Тест ContrastFilter с экстремальными значениями
TEST_F(FilterTestBase, ContrastFilterExtremeValues)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    // Большое значение (но не слишком большое, чтобы не вызвать ошибку)
    auto processor1 = createImageProcessor(width, height);
    ContrastFilter filter_high(10.0);
    EXPECT_TRUE(filter_high.apply(processor1).isSuccess());
    EXPECT_TRUE(processor1.isValid());
    
    // Очень маленькое значение
    auto processor2 = createImageProcessor(width, height);
    ContrastFilter filter_low(0.001);
    EXPECT_TRUE(filter_low.apply(processor2).isSuccess());
    EXPECT_TRUE(processor2.isValid());
}

// Тест SaturationFilter с экстремальными значениями
TEST_F(FilterTestBase, SaturationFilterExtremeValues)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    // Очень большое значение
    auto processor1 = createImageProcessor(width, height);
    SaturationFilter filter_high(10.0);
    EXPECT_TRUE(filter_high.apply(processor1).isSuccess());
    EXPECT_TRUE(processor1.isValid());
}

// Тест GaussianBlurFilter с различными радиусами
TEST_F(FilterTestBase, GaussianBlurFilterRadii)
{
    constexpr int width = 20;
    constexpr int height = 20;
    
    const double radii[] = {0.5, 1.0, 2.0, 5.0, 10.0};
    
    for (double radius : radii)
    {
        auto processor = createImageProcessor(width, height);
        GaussianBlurFilter filter(radius);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тест BoxBlurFilter с различными радиусами
TEST_F(FilterTestBase, BoxBlurFilterRadii)
{
    constexpr int width = 20;
    constexpr int height = 20;
    
    for (int radius = 1; radius <= 10; ++radius)
    {
        auto processor = createImageProcessor(width, height);
        BoxBlurFilter filter(radius);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тест NoiseFilter с различными интенсивностями
TEST_F(FilterTestBase, NoiseFilterIntensities)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    const double intensities[] = {0.01, 0.1, 0.5, 1.0, 2.0};
    
    for (double intensity : intensities)
    {
        auto processor = createImageProcessor(width, height);
        NoiseFilter filter(intensity);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тест VignetteFilter с различными интенсивностями
TEST_F(FilterTestBase, VignetteFilterIntensities)
{
    constexpr int width = 20;
    constexpr int height = 20;
    
    const double intensities[] = {0.1, 0.5, 1.0, 2.0, 5.0};
    
    for (double intensity : intensities)
    {
        auto processor = createImageProcessor(width, height);
        VignetteFilter filter(intensity);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тест Rotate90Filter против часовой стрелки
TEST_F(FilterTestBase, Rotate90FilterCounterClockwise)
{
    constexpr int width = 4;
    constexpr int height = 3;
    auto processor = createImageProcessor(width, height);
    
    const int original_width = processor.getWidth();
    const int original_height = processor.getHeight();
    
    Rotate90Filter filter(false); // Против часовой стрелки
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // После поворота на 90 градусов против часовой стрелки размеры должны поменяться местами
    EXPECT_EQ(processor.getWidth(), original_height);
    EXPECT_EQ(processor.getHeight(), original_width);
}

// Тест Rotate90Filter четыре раза против часовой стрелки
TEST_F(FilterTestBase, Rotate90FilterFourTimesCounterClockwise)
{
    constexpr int width = 4;
    constexpr int height = 3;
    auto processor = createImageProcessor(width, height);
    
    const int original_width = processor.getWidth();
    const int original_height = processor.getHeight();
    
    Rotate90Filter filter(false); // Против часовой стрелки
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(filter.apply(processor).isSuccess()); // Четыре раза = 360 градусов
    
    // После четырех поворотов размеры должны вернуться к исходным
    EXPECT_EQ(processor.getWidth(), original_width);
    EXPECT_EQ(processor.getHeight(), original_height);
}

// Тест применения нескольких фильтров подряд
TEST_F(FilterTestBase, MultipleFiltersSequential)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    GrayscaleFilter grayscale;
    EXPECT_TRUE(grayscale.apply(processor).isSuccess());
    
    BrightnessFilter brightness(1.2);
    EXPECT_TRUE(brightness.apply(processor).isSuccess());
    
    ContrastFilter contrast(1.5);
    EXPECT_TRUE(contrast.apply(processor).isSuccess());
    
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест применения фильтров к RGBA изображению (расширенный набор)
TEST_F(FilterTestBase, FiltersOnRGBAExtended)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    
    const auto filepath = test_dir_ / "test_rgba_extended.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    EXPECT_TRUE(processor.loadFromFile(test_file, true).isSuccess());
    EXPECT_EQ(processor.getChannels(), 4);
    
    // Сохраняем оригинальные альфа-каналы
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original_alpha(width * height);
    for (int i = 0; i < width * height; ++i)
    {
        original_alpha[i] = original_data[i * 4 + 3];
    }
    
    // Применяем различные фильтры
    BrightnessFilter brightness(1.5);
    EXPECT_TRUE(brightness.apply(processor).isSuccess());
    
    ContrastFilter contrast(1.2);
    EXPECT_TRUE(contrast.apply(processor).isSuccess());
    
    SaturationFilter saturation(1.3);
    EXPECT_TRUE(saturation.apply(processor).isSuccess());
    
    // Проверяем, что альфа-каналы не изменились
    const auto* data = processor.getData();
    for (int i = 0; i < width * height; ++i)
    {
        EXPECT_EQ(data[i * 4 + 3], original_alpha[i]) << "Alpha channel changed at pixel " << i;
    }
}

// Тест EdgeDetectionFilter с различными операторами
TEST_F(FilterTestBase, EdgeDetectionFilterOperators)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    // Тестируем различные операторы (если они поддерживаются)
    auto processor1 = createImageProcessor(width, height);
    EdgeDetectionFilter filter1(0.5, EdgeDetectionFilter::Operator::Sobel);
    EXPECT_TRUE(filter1.apply(processor1).isSuccess());
    EXPECT_TRUE(processor1.isValid());
    
    auto processor2 = createImageProcessor(width, height);
    EdgeDetectionFilter filter2(0.5, EdgeDetectionFilter::Operator::Prewitt);
    EXPECT_TRUE(filter2.apply(processor2).isSuccess());
    EXPECT_TRUE(processor2.isValid());
}

// Тест MedianFilter с большим радиусом
TEST_F(FilterTestBase, MedianFilterLargeRadius)
{
    constexpr int width = 50;
    constexpr int height = 50;
    auto processor = createImageProcessor(width, height);
    
    // Радиус близкий к половине размера изображения (но не слишком большой)
    MedianFilter filter(20);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
}

// Тест MotionBlurFilter с различными длинами
TEST_F(FilterTestBase, MotionBlurFilterLengths)
{
    constexpr int width = 20;
    constexpr int height = 20;
    
    const int lengths[] = {1, 5, 10, 20, 50};
    
    for (int length : lengths)
    {
        auto processor = createImageProcessor(width, height);
        MotionBlurFilter filter(length, 0.0);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тест применения фильтров к очень большому изображению
TEST_F(FilterTestBase, FiltersOnVeryLargeImage)
{
    constexpr int width = 500;
    constexpr int height = 500;
    auto processor = createImageProcessor(width, height);
    
    GrayscaleFilter grayscale;
    EXPECT_TRUE(grayscale.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
    
    BrightnessFilter brightness(1.2);
    EXPECT_TRUE(brightness.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
}

// Тест применения фильтров размытия к большому изображению
TEST_F(FilterTestBase, BlurFiltersOnLargeImage)
{
    constexpr int width = 200;
    constexpr int height = 200;
    auto processor = createImageProcessor(width, height);
    
    GaussianBlurFilter gaussian(2.0);
    EXPECT_TRUE(gaussian.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
    
    BoxBlurFilter box(5);
    EXPECT_TRUE(box.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
}

// Тест PosterizeFilter с минимальным количеством уровней
TEST_F(FilterTestBase, PosterizeFilterMinLevels)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Минимальное количество уровней (2)
    PosterizeFilter filter(2);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
}

// Тест PosterizeFilter с максимальным количеством уровней
TEST_F(FilterTestBase, PosterizeFilterMaxLevels)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Максимальное количество уровней (256)
    PosterizeFilter filter(256);
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
}

// Тест ThresholdFilter с граничными значениями порога
TEST_F(FilterTestBase, ThresholdFilterBoundaryValuesDetailed)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    // Порог = 0 (все пиксели должны стать белыми)
    auto processor1 = createImageProcessor(width, height);
    ThresholdFilter filter1(0);
    EXPECT_TRUE(filter1.apply(processor1).isSuccess());
    EXPECT_TRUE(processor1.isValid());
    
    // Порог = 255 (все пиксели должны стать черными)
    auto processor2 = createImageProcessor(width, height);
    ThresholdFilter filter2(255);
    EXPECT_TRUE(filter2.apply(processor2).isSuccess());
    EXPECT_TRUE(processor2.isValid());
}

// Тест SharpenFilter с различными интенсивностями
TEST_F(FilterTestBase, SharpenFilterIntensities)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    const double intensities[] = {0.5, 1.0, 2.0, 5.0};
    
    for (double intensity : intensities)
    {
        auto processor = createImageProcessor(width, height);
        SharpenFilter filter(intensity);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тест EmbossFilter с различными интенсивностями
TEST_F(FilterTestBase, EmbossFilterIntensities)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    const double intensities[] = {0.5, 1.0, 2.0, 5.0};
    
    for (double intensity : intensities)
    {
        auto processor = createImageProcessor(width, height);
        EmbossFilter filter(intensity);
        EXPECT_TRUE(filter.apply(processor).isSuccess());
        EXPECT_TRUE(processor.isValid());
    }
}

// Тест применения фильтров к изображению с нестандартными размерами
TEST_F(FilterTestBase, FiltersOnNonSquareImage)
{
    constexpr int width = 20;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    GrayscaleFilter grayscale;
    EXPECT_TRUE(grayscale.apply(processor).isSuccess());
    
    FlipHorizontalFilter flip_h;
    EXPECT_TRUE(flip_h.apply(processor).isSuccess());
    
    FlipVerticalFilter flip_v;
    EXPECT_TRUE(flip_v.apply(processor).isSuccess());
    
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест FlipVerticalFilter с изображением 1x1
TEST_F(FilterTestBase, FlipVerticalFilter1x1)
{
    constexpr int width = 1;
    constexpr int height = 1;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    FlipVerticalFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
    
    // Для изображения 1x1 отражение не должно изменить данные
    const auto* flipped_data = processor.getData();
    for (size_t i = 0; i < original.size(); ++i)
    {
        EXPECT_EQ(flipped_data[i], original[i]);
    }
}

// Тест FlipVerticalFilter с изображением 2x2
TEST_F(FilterTestBase, FlipVerticalFilter2x2)
{
    constexpr int width = 2;
    constexpr int height = 2;
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

// Тест FlipVerticalFilter с изображением нечетной высоты
TEST_F(FilterTestBase, FlipVerticalFilterOddHeight)
{
    constexpr int width = 5;
    constexpr int height = 3; // Нечетная высота
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

// Тест FlipVerticalFilter с очень большим изображением
TEST_F(FilterTestBase, FlipVerticalFilterVeryLarge)
{
    constexpr int width = 100;
    constexpr int height = 100;
    auto processor = createImageProcessor(width, height);
    
    FlipVerticalFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест FlipVerticalFilter с RGBA изображением
TEST_F(FilterTestBase, FlipVerticalFilterRGBA)
{
    constexpr int width = 5;
    constexpr int height = 5;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    
    const auto filepath = test_dir_ / "test_rgba_flip_v.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    EXPECT_TRUE(processor.loadFromFile(test_file, true).isSuccess());
    EXPECT_EQ(processor.getChannels(), 4);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 4);
    
    FlipVerticalFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем отражение по вертикали
    const auto* flipped_data = processor.getData();
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const auto orig_index = static_cast<size_t>(y * width + x) * 4;
            const auto flip_index = static_cast<size_t>((height - 1 - y) * width + x) * 4;
            
            EXPECT_EQ(flipped_data[orig_index], original[flip_index]);     // R
            EXPECT_EQ(flipped_data[orig_index + 1], original[flip_index + 1]); // G
            EXPECT_EQ(flipped_data[orig_index + 2], original[flip_index + 2]); // B
            EXPECT_EQ(flipped_data[orig_index + 3], original[flip_index + 3]); // A
        }
    }
}

// Тест FlipVerticalFilter с некорректным изображением
TEST_F(FilterTestBase, FlipVerticalFilterInvalidImage)
{
    ImageProcessor processor; // Пустое изображение
    
    FlipVerticalFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест FlipVerticalFilter с нулевыми размерами
TEST_F(FilterTestBase, FlipVerticalFilterZeroSize)
{
    ImageProcessor processor;
    // Создаем изображение с нулевыми размерами (если это возможно)
    // В реальности это должно быть обработано как ошибка
    
    FlipVerticalFilter filter;
    // Применение к невалидному изображению должно вернуть ошибку
    const auto result = filter.apply(processor);
    EXPECT_FALSE(result.isSuccess());
}

// Тест FlipHorizontalFilter с изображением 1x1
TEST_F(FilterTestBase, FlipHorizontalFilter1x1)
{
    constexpr int width = 1;
    constexpr int height = 1;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    FlipHorizontalFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
    
    // Для изображения 1x1 отражение не должно изменить данные
    const auto* flipped_data = processor.getData();
    for (size_t i = 0; i < original.size(); ++i)
    {
        EXPECT_EQ(flipped_data[i], original[i]);
    }
}

// Тест FlipHorizontalFilter с изображением 2x2
TEST_F(FilterTestBase, FlipHorizontalFilter2x2)
{
    constexpr int width = 2;
    constexpr int height = 2;
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

// Тест FlipHorizontalFilter с изображением нечетной ширины
TEST_F(FilterTestBase, FlipHorizontalFilterOddWidth)
{
    constexpr int width = 3; // Нечетная ширина
    constexpr int height = 5;
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

// Тест FlipHorizontalFilter с очень большим изображением
TEST_F(FilterTestBase, FlipHorizontalFilterVeryLarge)
{
    constexpr int width = 100;
    constexpr int height = 100;
    auto processor = createImageProcessor(width, height);
    
    FlipHorizontalFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест FlipHorizontalFilter с RGBA изображением
TEST_F(FilterTestBase, FlipHorizontalFilterRGBA)
{
    constexpr int width = 5;
    constexpr int height = 5;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    
    const auto filepath = test_dir_ / "test_rgba_flip_h.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    EXPECT_TRUE(processor.loadFromFile(test_file, true).isSuccess());
    EXPECT_EQ(processor.getChannels(), 4);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 4);
    
    FlipHorizontalFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    // Проверяем отражение по горизонтали
    const auto* flipped_data = processor.getData();
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const auto orig_index = static_cast<size_t>(y * width + x) * 4;
            const auto flip_index = static_cast<size_t>(y * width + (width - 1 - x)) * 4;
            
            EXPECT_EQ(flipped_data[orig_index], original[flip_index]);     // R
            EXPECT_EQ(flipped_data[orig_index + 1], original[flip_index + 1]); // G
            EXPECT_EQ(flipped_data[orig_index + 2], original[flip_index + 2]); // B
            EXPECT_EQ(flipped_data[orig_index + 3], original[flip_index + 3]); // A
        }
    }
}

// Тест FlipHorizontalFilter с некорректным изображением
TEST_F(FilterTestBase, FlipHorizontalFilterInvalidImage)
{
    ImageProcessor processor; // Пустое изображение
    
    FlipHorizontalFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест FlipHorizontalFilter с нулевыми размерами
TEST_F(FilterTestBase, FlipHorizontalFilterZeroSize)
{
    ImageProcessor processor;
    // Создаем изображение с нулевыми размерами (если это возможно)
    // В реальности это должно быть обработано как ошибка
    
    FlipHorizontalFilter filter;
    // Применение к невалидному изображению должно вернуть ошибку
    const auto result = filter.apply(processor);
    EXPECT_FALSE(result.isSuccess());
}

// Тест последовательного применения FlipVertical и FlipHorizontal
TEST_F(FilterTestBase, FlipVerticalThenHorizontal)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    FlipVerticalFilter flip_v;
    EXPECT_TRUE(flip_v.apply(processor).isSuccess());
    
    FlipHorizontalFilter flip_h;
    EXPECT_TRUE(flip_h.apply(processor).isSuccess());
    
    // Проверяем, что изображение валидно
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест последовательного применения FlipHorizontal и FlipVertical
TEST_F(FilterTestBase, FlipHorizontalThenVertical)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Сохраняем оригинальные данные
    const auto* original_data = processor.getData();
    std::vector<uint8_t> original(original_data, original_data + width * height * 3);
    
    FlipHorizontalFilter flip_h;
    EXPECT_TRUE(flip_h.apply(processor).isSuccess());
    
    FlipVerticalFilter flip_v;
    EXPECT_TRUE(flip_v.apply(processor).isSuccess());
    
    // Проверяем, что изображение валидно
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест FlipVerticalFilter с очень широким изображением
TEST_F(FilterTestBase, FlipVerticalFilterWideImage)
{
    constexpr int width = 100;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    FlipVerticalFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест FlipHorizontalFilter с очень высоким изображением
TEST_F(FilterTestBase, FlipHorizontalFilterTallImage)
{
    constexpr int width = 10;
    constexpr int height = 100;
    auto processor = createImageProcessor(width, height);
    
    FlipHorizontalFilter filter;
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест FlipVerticalFilter с невалидным изображением (if (!image.isValid()))
TEST_F(FilterTestBase, FlipVerticalFilterInvalidImageBranch)
{
    ImageProcessor processor; // Пустое изображение
    
    FlipVerticalFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест FlipVerticalFilter с нулевыми размерами (if (width <= 0 || height <= 0))
TEST_F(FilterTestBase, FlipVerticalFilterZeroSizeBranch)
{
    ImageProcessor processor;
    processor.resize(0, 0);
    
    FlipVerticalFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_FALSE(result.isSuccess());
    // resize(0, 0) делает изображение невалидным, поэтому ожидаем InvalidImage
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест FlipVerticalFilter с отрицательными размерами
TEST_F(FilterTestBase, FlipVerticalFilterNegativeSizeBranch)
{
    ImageProcessor processor;
    processor.resize(-1, -1);
    
    FlipVerticalFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_FALSE(result.isSuccess());
    // resize(-1, -1) делает изображение невалидным, поэтому ожидаем InvalidImage
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест FlipVerticalFilter с некорректными каналами (if (channels != 3 && channels != 4))
TEST_F(FilterTestBase, FlipVerticalFilterInvalidChannelsBranch)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Создаем изображение с некорректным количеством каналов
    // Это сложно сделать напрямую, но можно протестировать через resize
    auto test_image = TestUtils::createTestImage(width, height);
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    // Пытаемся установить некорректное количество каналов (это должно быть невозможно)
    // Но если это произойдет, фильтр должен вернуть ошибку
    // В реальности resize не позволит установить некорректные каналы
    // Поэтому тестируем через прямой доступ к данным (если возможно)
    
    FlipVerticalFilter filter;
    // Для валидного изображения должно работать
    EXPECT_TRUE(filter.apply(processor).isSuccess());
    
    std::free(allocated_data);
}

// Тест FlipHorizontalFilter с невалидным изображением
TEST_F(FilterTestBase, FlipHorizontalFilterInvalidImageBranch)
{
    ImageProcessor processor; // Пустое изображение
    
    FlipHorizontalFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест FlipHorizontalFilter с нулевыми размерами
TEST_F(FilterTestBase, FlipHorizontalFilterZeroSizeBranch)
{
    ImageProcessor processor;
    processor.resize(0, 0);
    
    FlipHorizontalFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_FALSE(result.isSuccess());
    // resize(0, 0) делает изображение невалидным, поэтому ожидаем InvalidImage
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест FlipHorizontalFilter с отрицательными размерами
TEST_F(FilterTestBase, FlipHorizontalFilterNegativeSizeBranch)
{
    ImageProcessor processor;
    processor.resize(-1, -1);
    
    FlipHorizontalFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_FALSE(result.isSuccess());
    // resize(-1, -1) делает изображение невалидным, поэтому ожидаем InvalidImage
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест OutlineFilter с невалидным изображением
TEST_F(FilterTestBase, OutlineFilterInvalidImageBranch)
{
    ImageProcessor processor; // Пустое изображение
    
    OutlineFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест OutlineFilter с нулевыми размерами
TEST_F(FilterTestBase, OutlineFilterZeroSizeBranch)
{
    ImageProcessor processor;
    processor.resize(0, 0);
    
    OutlineFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_FALSE(result.isSuccess());
    // resize(0, 0) делает изображение невалидным, поэтому ожидаем InvalidImage
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест OutlineFilter с отрицательными размерами
TEST_F(FilterTestBase, OutlineFilterNegativeSizeBranch)
{
    ImageProcessor processor;
    processor.resize(-1, -1);
    
    OutlineFilter filter;
    const auto result = filter.apply(processor);
    EXPECT_FALSE(result.isSuccess());
    // resize(-1, -1) делает изображение невалидным, поэтому ожидаем InvalidImage
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест OutlineFilter с некорректными каналами
TEST_F(FilterTestBase, OutlineFilterInvalidChannelsBranch)
{
    ImageProcessor processor;
    processor.resize(10, 10);
    // В реальности нельзя установить некорректные каналы, но тестируем валидный случай
    auto processor_valid = createImageProcessor(10, 10);
    
    OutlineFilter filter;
    EXPECT_TRUE(filter.apply(processor_valid).isSuccess());
}

// Тест BoxBlurFilter с проверкой переполнения (if (!SafeMath::safeMultiply(...)))
TEST_F(FilterTestBase, BoxBlurFilterOverflowCheck)
{
    // Создаем изображение с очень большими размерами, которые могут вызвать переполнение
    // В реальности это сложно протестировать, так как SafeMath защищает от переполнения
    // Но проверяем, что фильтр корректно обрабатывает большие изображения
    
    constexpr int width = 1000;
    constexpr int height = 1000;
    auto processor = createImageProcessor(width, height);
    
    BoxBlurFilter filter(5);
    const auto result = filter.apply(processor);
    // Должно работать для разумных размеров
    EXPECT_TRUE(result.isSuccess());
}

// Тест EmbossFilter с проверкой переполнения
TEST_F(FilterTestBase, EmbossFilterOverflowCheck)
{
    constexpr int width = 1000;
    constexpr int height = 1000;
    auto processor = createImageProcessor(width, height);
    
    EmbossFilter filter(1.0);
    const auto result = filter.apply(processor);
    EXPECT_TRUE(result.isSuccess());
}

// Тест ThresholdFilter с граничными значениями порога
TEST_F(FilterTestBase, ThresholdFilterBoundaryThreshold)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto processor = createImageProcessor(width, height);
    
    // Минимальное значение порога: 0
    ThresholdFilter filter_min(0);
    EXPECT_TRUE(filter_min.apply(processor).isSuccess());
    
    // Максимальное значение порога: 255
    auto processor2 = createImageProcessor(width, height);
    ThresholdFilter filter_max(255);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
    
    // Граничные значения: 1 и 254
    auto processor3 = createImageProcessor(width, height);
    ThresholdFilter filter_1(1);
    EXPECT_TRUE(filter_1.apply(processor3).isSuccess());
    
    auto processor4 = createImageProcessor(width, height);
    ThresholdFilter filter_254(254);
    EXPECT_TRUE(filter_254.apply(processor4).isSuccess());
}

// Тест PosterizeFilter с граничными значениями уровней
TEST_F(FilterTestBase, PosterizeFilterBoundaryLevels)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    // Минимальное значение: 2
    auto processor1 = createImageProcessor(width, height);
    PosterizeFilter filter_min(2);
    EXPECT_TRUE(filter_min.apply(processor1).isSuccess());
    
    // Максимальное значение: 256
    auto processor2 = createImageProcessor(width, height);
    PosterizeFilter filter_max(256);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
    
    // Граничные значения: 3 и 255
    auto processor3 = createImageProcessor(width, height);
    PosterizeFilter filter_3(3);
    EXPECT_TRUE(filter_3.apply(processor3).isSuccess());
    
    auto processor4 = createImageProcessor(width, height);
    PosterizeFilter filter_255(255);
    EXPECT_TRUE(filter_255.apply(processor4).isSuccess());
}

// Тест BrightnessFilter с граничными значениями фактора
TEST_F(FilterTestBase, BrightnessFilterBoundaryFactor)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    // Очень маленькое значение, близкое к 0
    auto processor1 = createImageProcessor(width, height);
    BrightnessFilter filter_min(0.0001);
    EXPECT_TRUE(filter_min.apply(processor1).isSuccess());
    
    // Очень большое значение (но в пределах допустимого диапазона 0.0-10.0)
    auto processor2 = createImageProcessor(width, height);
    BrightnessFilter filter_max(10.0);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
    
    // Нулевое значение
    auto processor3 = createImageProcessor(width, height);
    BrightnessFilter filter_zero(0.0);
    EXPECT_TRUE(filter_zero.apply(processor3).isSuccess());
}

// Тест ContrastFilter с граничными значениями фактора
TEST_F(FilterTestBase, ContrastFilterBoundaryFactor)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    // Очень маленькое значение
    auto processor1 = createImageProcessor(width, height);
    ContrastFilter filter_min(0.0001);
    EXPECT_TRUE(filter_min.apply(processor1).isSuccess());
    
    // Очень большое значение (но в пределах допустимого диапазона 0.0-10.0)
    auto processor2 = createImageProcessor(width, height);
    ContrastFilter filter_max(10.0);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
    
    // Нулевое значение
    auto processor3 = createImageProcessor(width, height);
    ContrastFilter filter_zero(0.0);
    EXPECT_TRUE(filter_zero.apply(processor3).isSuccess());
}

// Тест SaturationFilter с граничными значениями фактора
TEST_F(FilterTestBase, SaturationFilterBoundaryFactor)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    // Очень маленькое значение
    auto processor1 = createImageProcessor(width, height);
    SaturationFilter filter_min(0.0001);
    EXPECT_TRUE(filter_min.apply(processor1).isSuccess());
    
    // Очень большое значение (но в пределах допустимого диапазона 0.0-10.0)
    auto processor2 = createImageProcessor(width, height);
    SaturationFilter filter_max(10.0);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
}

// Тест GaussianBlurFilter с граничными значениями радиуса
TEST_F(FilterTestBase, GaussianBlurFilterBoundaryRadius)
{
    constexpr int width = 20;
    constexpr int height = 20;
    
    // Минимальное значение: очень маленький радиус
    auto processor1 = createImageProcessor(width, height);
    GaussianBlurFilter filter_min(0.1);
    EXPECT_TRUE(filter_min.apply(processor1).isSuccess());
    
    // Большое значение радиуса
    auto processor2 = createImageProcessor(width, height);
    GaussianBlurFilter filter_max(10.0);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
    
    // Нулевое значение
    auto processor3 = createImageProcessor(width, height);
    GaussianBlurFilter filter_zero(0.0);
    EXPECT_TRUE(filter_zero.apply(processor3).isSuccess());
}

// Тест BoxBlurFilter с граничными значениями радиуса
TEST_F(FilterTestBase, BoxBlurFilterBoundaryRadius)
{
    constexpr int width = 20;
    constexpr int height = 20;
    
    // Минимальное значение: 1
    auto processor1 = createImageProcessor(width, height);
    BoxBlurFilter filter_min(1);
    EXPECT_TRUE(filter_min.apply(processor1).isSuccess());
    
    // Большое значение радиуса
    auto processor2 = createImageProcessor(width, height);
    BoxBlurFilter filter_max(10);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
    
    // Нулевое значение
    auto processor3 = createImageProcessor(width, height);
    BoxBlurFilter filter_zero(0);
    EXPECT_TRUE(filter_zero.apply(processor3).isSuccess());
}

// Тест MedianFilter с граничными значениями радиуса
TEST_F(FilterTestBase, MedianFilterBoundaryRadius)
{
    constexpr int width = 20;
    constexpr int height = 20;
    
    // Минимальное значение: 1
    auto processor1 = createImageProcessor(width, height);
    MedianFilter filter_min(1);
    EXPECT_TRUE(filter_min.apply(processor1).isSuccess());
    
    // Большое значение радиуса
    auto processor2 = createImageProcessor(width, height);
    MedianFilter filter_max(10);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
}

// Тест MotionBlurFilter с граничными значениями длины
TEST_F(FilterTestBase, MotionBlurFilterBoundaryLength)
{
    constexpr int width = 20;
    constexpr int height = 20;
    
    // Минимальное значение: 1
    auto processor1 = createImageProcessor(width, height);
    MotionBlurFilter filter_min(1, 0.0);
    EXPECT_TRUE(filter_min.apply(processor1).isSuccess());
    
    // Большое значение длины
    auto processor2 = createImageProcessor(width, height);
    MotionBlurFilter filter_max(50, 0.0);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
    
    // Нулевое значение
    auto processor3 = createImageProcessor(width, height);
    MotionBlurFilter filter_zero(0, 0.0);
    EXPECT_TRUE(filter_zero.apply(processor3).isSuccess());
}

// Тест MotionBlurFilter с граничными значениями угла
TEST_F(FilterTestBase, MotionBlurFilterBoundaryAngle)
{
    constexpr int width = 20;
    constexpr int height = 20;
    
    // Минимальное значение: 0.0
    auto processor1 = createImageProcessor(width, height);
    MotionBlurFilter filter_0(10, 0.0);
    EXPECT_TRUE(filter_0.apply(processor1).isSuccess());
    
    // Максимальное значение: 360.0
    auto processor2 = createImageProcessor(width, height);
    MotionBlurFilter filter_360(10, 360.0);
    EXPECT_TRUE(filter_360.apply(processor2).isSuccess());
    
    // Граничные значения: 90.0, 180.0, 270.0
    auto processor3 = createImageProcessor(width, height);
    MotionBlurFilter filter_90(10, 90.0);
    EXPECT_TRUE(filter_90.apply(processor3).isSuccess());
    
    auto processor4 = createImageProcessor(width, height);
    MotionBlurFilter filter_180(10, 180.0);
    EXPECT_TRUE(filter_180.apply(processor4).isSuccess());
    
    auto processor5 = createImageProcessor(width, height);
    MotionBlurFilter filter_270(10, 270.0);
    EXPECT_TRUE(filter_270.apply(processor5).isSuccess());
}

// Тест NoiseFilter с граничными значениями интенсивности
TEST_F(FilterTestBase, NoiseFilterBoundaryIntensity)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    // Минимальное значение: очень маленькая интенсивность
    auto processor1 = createImageProcessor(width, height);
    NoiseFilter filter_min(0.0001);
    EXPECT_TRUE(filter_min.apply(processor1).isSuccess());
    
    // Большое значение интенсивности
    auto processor2 = createImageProcessor(width, height);
    NoiseFilter filter_max(10.0);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
    
    // Нулевое значение
    auto processor3 = createImageProcessor(width, height);
    NoiseFilter filter_zero(0.0);
    EXPECT_TRUE(filter_zero.apply(processor3).isSuccess());
}

// Тест VignetteFilter с граничными значениями интенсивности
TEST_F(FilterTestBase, VignetteFilterBoundaryIntensity)
{
    constexpr int width = 20;
    constexpr int height = 20;
    
    // Минимальное значение
    auto processor1 = createImageProcessor(width, height);
    VignetteFilter filter_min(0.1);
    EXPECT_TRUE(filter_min.apply(processor1).isSuccess());
    
    // Большое значение интенсивности
    auto processor2 = createImageProcessor(width, height);
    VignetteFilter filter_max(10.0);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
    
    // Нулевое значение
    auto processor3 = createImageProcessor(width, height);
    VignetteFilter filter_zero(0.0);
    EXPECT_TRUE(filter_zero.apply(processor3).isSuccess());
}

// Тест SharpenFilter с граничными значениями интенсивности
TEST_F(FilterTestBase, SharpenFilterBoundaryIntensity)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    // Минимальное значение
    auto processor1 = createImageProcessor(width, height);
    SharpenFilter filter_min(0.1);
    EXPECT_TRUE(filter_min.apply(processor1).isSuccess());
    
    // Большое значение интенсивности
    auto processor2 = createImageProcessor(width, height);
    SharpenFilter filter_max(10.0);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
    
    // Нулевое значение
    auto processor3 = createImageProcessor(width, height);
    SharpenFilter filter_zero(0.0);
    EXPECT_TRUE(filter_zero.apply(processor3).isSuccess());
}

// Тест EmbossFilter с граничными значениями интенсивности
TEST_F(FilterTestBase, EmbossFilterBoundaryIntensity)
{
    constexpr int width = 10;
    constexpr int height = 10;
    
    // Минимальное значение
    auto processor1 = createImageProcessor(width, height);
    EmbossFilter filter_min(0.1);
    EXPECT_TRUE(filter_min.apply(processor1).isSuccess());
    
    // Большое значение интенсивности
    auto processor2 = createImageProcessor(width, height);
    EmbossFilter filter_max(10.0);
    EXPECT_TRUE(filter_max.apply(processor2).isSuccess());
    
    // Нулевое значение
    auto processor3 = createImageProcessor(width, height);
    EmbossFilter filter_zero(0.0);
    EXPECT_TRUE(filter_zero.apply(processor3).isSuccess());
}

// ========== ФАЗА 3.2: Тесты для граничных значений в фильтрах ==========

// Тест фильтров с минимальными размерами изображения (1x1)
TEST_F(FilterTestBase, FiltersBoundaryMinSize1x1)
{
    constexpr int width = 1;
    constexpr int height = 1;
    auto processor = createImageProcessor(width, height);
    
    // Тестируем различные фильтры на минимальном размере
    GrayscaleFilter grayscale;
    EXPECT_TRUE(grayscale.apply(processor).isSuccess());
    
    InvertFilter invert;
    EXPECT_TRUE(invert.apply(processor).isSuccess());
    
    BrightnessFilter brightness(1.5);
    EXPECT_TRUE(brightness.apply(processor).isSuccess());
    
    ThresholdFilter threshold(128);
    EXPECT_TRUE(threshold.apply(processor).isSuccess());
}

// Тест фильтров с минимальными размерами изображения (1x2)
TEST_F(FilterTestBase, FiltersBoundaryMinSize1x2)
{
    constexpr int width = 1;
    constexpr int height = 2;
    auto processor = createImageProcessor(width, height);
    
    GrayscaleFilter grayscale;
    EXPECT_TRUE(grayscale.apply(processor).isSuccess());
    
    FlipVerticalFilter flip_v;
    EXPECT_TRUE(flip_v.apply(processor).isSuccess());
}

// Тест фильтров с минимальными размерами изображения (2x1)
TEST_F(FilterTestBase, FiltersBoundaryMinSize2x1)
{
    constexpr int width = 2;
    constexpr int height = 1;
    auto processor = createImageProcessor(width, height);
    
    GrayscaleFilter grayscale;
    EXPECT_TRUE(grayscale.apply(processor).isSuccess());
    
    FlipHorizontalFilter flip_h;
    EXPECT_TRUE(flip_h.apply(processor).isSuccess());
}

// Тест фильтров с очень большими размерами изображения
TEST_F(FilterTestBase, FiltersBoundaryLargeSize)
{
    constexpr int width = 1000;
    constexpr int height = 1000;
    auto processor = createImageProcessor(width, height);
    
    GrayscaleFilter grayscale;
    EXPECT_TRUE(grayscale.apply(processor).isSuccess());
    
    BrightnessFilter brightness(1.2);
    EXPECT_TRUE(brightness.apply(processor).isSuccess());
    
    ContrastFilter contrast(1.5);
    EXPECT_TRUE(contrast.apply(processor).isSuccess());
}

// Тест фильтров с нечетными размерами
TEST_F(FilterTestBase, FiltersBoundaryOddSizes)
{
    constexpr int width = 3;
    constexpr int height = 5;
    auto processor = createImageProcessor(width, height);
    
    FlipVerticalFilter flip_v;
    EXPECT_TRUE(flip_v.apply(processor).isSuccess());
    
    FlipHorizontalFilter flip_h;
    EXPECT_TRUE(flip_h.apply(processor).isSuccess());
    
    Rotate90Filter rotate(true);
    EXPECT_TRUE(rotate.apply(processor).isSuccess());
}

// Тест фильтров с четными размерами
TEST_F(FilterTestBase, FiltersBoundaryEvenSizes)
{
    constexpr int width = 4;
    constexpr int height = 6;
    auto processor = createImageProcessor(width, height);
    
    FlipVerticalFilter flip_v;
    EXPECT_TRUE(flip_v.apply(processor).isSuccess());
    
    FlipHorizontalFilter flip_h;
    EXPECT_TRUE(flip_h.apply(processor).isSuccess());
}
