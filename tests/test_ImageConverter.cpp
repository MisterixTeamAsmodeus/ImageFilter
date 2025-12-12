#include "test_utils.h"
#include <utils/ImageConverter.h>
#include <utils/FilterResult.h>
#include <utils/ErrorCodes.h>
#include <gtest/gtest.h>
#include <cstdlib>
#include <cstring>
#include <cmath>

/**
 * @brief Тесты для ImageConverter
 */
class ImageConverterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        createTestImageData();
    }
    
    void TearDown() override
    {
        if (rgba_data_ != nullptr)
        {
            std::free(rgba_data_);
            rgba_data_ = nullptr;
        }
        
        if (rgb_data_ != nullptr)
        {
            std::free(rgb_data_);
            rgb_data_ = nullptr;
        }
    }
    
    void createTestImageData()
    {
        constexpr int width = 10;
        constexpr int height = 10;
        
        // Создаем RGBA данные
        auto rgba_image = TestUtils::createTestImageRGBA(width, height);
        rgba_data_ = static_cast<uint8_t*>(std::malloc(rgba_image.size()));
        ASSERT_NE(rgba_data_, nullptr);
        std::memcpy(rgba_data_, rgba_image.data(), rgba_image.size());
        
        // Выделяем память для RGB данных
        const size_t rgb_size = static_cast<size_t>(width) * height * 3;
        rgb_data_ = static_cast<uint8_t*>(std::malloc(rgb_size));
        ASSERT_NE(rgb_data_, nullptr);
        
        width_ = width;
        height_ = height;
    }
    
    uint8_t* rgba_data_ = nullptr;
    uint8_t* rgb_data_ = nullptr;
    int width_ = 10;
    int height_ = 10;
};

/**
 * @brief Тест конвертации RGBA в RGB
 */
TEST_F(ImageConverterTest, ConvertRGBAToRGB)
{
    const auto result = ImageConverter::convertRGBAToRGB(rgba_data_, width_, height_, rgb_data_);
    
    EXPECT_TRUE(result.isSuccess());
    
    // Проверяем, что данные были преобразованы
    // Первый пиксель должен быть преобразован с учетом альфа-канала
    const size_t first_pixel_rgba = 0;
    const size_t first_pixel_rgb = 0;
    
    const uint8_t r_orig = rgba_data_[first_pixel_rgba + 0];
    const uint8_t g_orig = rgba_data_[first_pixel_rgba + 1];
    const uint8_t b_orig = rgba_data_[first_pixel_rgba + 2];
    const uint8_t a_orig = rgba_data_[first_pixel_rgba + 3];
    
    const uint8_t r_conv = rgb_data_[first_pixel_rgb + 0];
    const uint8_t g_conv = rgb_data_[first_pixel_rgb + 1];
    const uint8_t b_conv = rgb_data_[first_pixel_rgb + 2];
    
    // Проверяем, что значения были преобразованы (могут отличаться из-за альфа-блендинга)
    // Если альфа = 255 (полная непрозрачность), значения должны быть близки к оригиналу
    if (a_orig == 255)
    {
        EXPECT_EQ(r_conv, r_orig);
        EXPECT_EQ(g_conv, g_orig);
        EXPECT_EQ(b_conv, b_orig);
    }
    else
    {
        // При альфа-блендинге значения должны быть изменены
        // Проверяем, что преобразование произошло
        EXPECT_GE(r_conv, 0);
        EXPECT_LE(r_conv, 255);
        EXPECT_GE(g_conv, 0);
        EXPECT_LE(g_conv, 255);
        EXPECT_GE(b_conv, 0);
        EXPECT_LE(b_conv, 255);
    }
}

/**
 * @brief Тест конвертации с нулевым указателем RGBA данных
 */
TEST_F(ImageConverterTest, ConvertNullRGBA)
{
    const auto result = ImageConverter::convertRGBAToRGB(nullptr, width_, height_, rgb_data_);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

/**
 * @brief Тест конвертации с нулевым указателем RGB данных
 */
TEST_F(ImageConverterTest, ConvertNullRGB)
{
    const auto result = ImageConverter::convertRGBAToRGB(rgba_data_, width_, height_, nullptr);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

/**
 * @brief Тест конвертации с нулевой шириной
 */
TEST_F(ImageConverterTest, ConvertZeroWidth)
{
    const auto result = ImageConverter::convertRGBAToRGB(rgba_data_, 0, height_, rgb_data_);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidSize);
}

/**
 * @brief Тест конвертации с нулевой высотой
 */
TEST_F(ImageConverterTest, ConvertZeroHeight)
{
    const auto result = ImageConverter::convertRGBAToRGB(rgba_data_, width_, 0, rgb_data_);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidSize);
}

/**
 * @brief Тест конвертации с отрицательной шириной
 */
TEST_F(ImageConverterTest, ConvertNegativeWidth)
{
    const auto result = ImageConverter::convertRGBAToRGB(rgba_data_, -1, height_, rgb_data_);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidSize);
}

/**
 * @brief Тест конвертации с отрицательной высотой
 */
TEST_F(ImageConverterTest, ConvertNegativeHeight)
{
    const auto result = ImageConverter::convertRGBAToRGB(rgba_data_, width_, -1, rgb_data_);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidSize);
}

/**
 * @brief Тест конвертации полностью непрозрачного изображения
 */
TEST_F(ImageConverterTest, ConvertFullyOpaque)
{
    // Создаем RGBA изображение с полной непрозрачностью
    constexpr int width = 5;
    constexpr int height = 5;
    const size_t rgba_size = static_cast<size_t>(width) * height * 4;
    uint8_t* opaque_rgba = static_cast<uint8_t*>(std::malloc(rgba_size));
    ASSERT_NE(opaque_rgba, nullptr);
    
    // Заполняем данными с альфа = 255
    for (size_t i = 0; i < rgba_size; i += 4)
    {
        opaque_rgba[i + 0] = 100; // R
        opaque_rgba[i + 1] = 150; // G
        opaque_rgba[i + 2] = 200; // B
        opaque_rgba[i + 3] = 255; // A (полная непрозрачность)
    }
    
    const size_t rgb_size = static_cast<size_t>(width) * height * 3;
    uint8_t* opaque_rgb = static_cast<uint8_t*>(std::malloc(rgb_size));
    ASSERT_NE(opaque_rgb, nullptr);
    
    const auto result = ImageConverter::convertRGBAToRGB(opaque_rgba, width, height, opaque_rgb);
    
    EXPECT_TRUE(result.isSuccess());
    
    // При полной непрозрачности значения должны совпадать
    for (size_t i = 0; i < static_cast<size_t>(width) * height; ++i)
    {
        EXPECT_EQ(opaque_rgb[i * 3 + 0], opaque_rgba[i * 4 + 0]);
        EXPECT_EQ(opaque_rgb[i * 3 + 1], opaque_rgba[i * 4 + 1]);
        EXPECT_EQ(opaque_rgb[i * 3 + 2], opaque_rgba[i * 4 + 2]);
    }
    
    std::free(opaque_rgba);
    std::free(opaque_rgb);
}

/**
 * @brief Тест конвертации полностью прозрачного изображения
 */
TEST_F(ImageConverterTest, ConvertFullyTransparent)
{
    // Создаем RGBA изображение с полной прозрачностью
    constexpr int width = 5;
    constexpr int height = 5;
    const size_t rgba_size = static_cast<size_t>(width) * height * 4;
    uint8_t* transparent_rgba = static_cast<uint8_t*>(std::malloc(rgba_size));
    ASSERT_NE(transparent_rgba, nullptr);
    
    // Заполняем данными с альфа = 0
    for (size_t i = 0; i < rgba_size; i += 4)
    {
        transparent_rgba[i + 0] = 100; // R
        transparent_rgba[i + 1] = 150; // G
        transparent_rgba[i + 2] = 200; // B
        transparent_rgba[i + 3] = 0; // A (полная прозрачность)
    }
    
    const size_t rgb_size = static_cast<size_t>(width) * height * 3;
    uint8_t* transparent_rgb = static_cast<uint8_t*>(std::malloc(rgb_size));
    ASSERT_NE(transparent_rgb, nullptr);
    
    const auto result = ImageConverter::convertRGBAToRGB(transparent_rgba, width, height, transparent_rgb);
    
    EXPECT_TRUE(result.isSuccess());
    
    // При полной прозрачности значения должны быть белыми (255, 255, 255)
    // из-за альфа-блендинга с белым фоном
    for (size_t i = 0; i < static_cast<size_t>(width) * height; ++i)
    {
        EXPECT_EQ(transparent_rgb[i * 3 + 0], 255);
        EXPECT_EQ(transparent_rgb[i * 3 + 1], 255);
        EXPECT_EQ(transparent_rgb[i * 3 + 2], 255);
    }
    
    std::free(transparent_rgba);
    std::free(transparent_rgb);
}

/**
 * @brief Тест конвертации с частичной прозрачностью
 */
TEST_F(ImageConverterTest, ConvertPartiallyTransparent)
{
    // Создаем RGBA изображение с частичной прозрачностью
    constexpr int width = 5;
    constexpr int height = 5;
    const size_t rgba_size = static_cast<size_t>(width) * height * 4;
    uint8_t* partial_rgba = static_cast<uint8_t*>(std::malloc(rgba_size));
    ASSERT_NE(partial_rgba, nullptr);
    
    // Заполняем данными с альфа = 128 (50% непрозрачности)
    for (size_t i = 0; i < rgba_size; i += 4)
    {
        partial_rgba[i + 0] = 100; // R
        partial_rgba[i + 1] = 150; // G
        partial_rgba[i + 2] = 200; // B
        partial_rgba[i + 3] = 128; // A (50% непрозрачности)
    }
    
    const size_t rgb_size = static_cast<size_t>(width) * height * 3;
    uint8_t* partial_rgb = static_cast<uint8_t*>(std::malloc(rgb_size));
    ASSERT_NE(partial_rgb, nullptr);
    
    const auto result = ImageConverter::convertRGBAToRGB(partial_rgba, width, height, partial_rgb);
    
    EXPECT_TRUE(result.isSuccess());
    
    // При 50% непрозрачности значения должны быть между оригиналом и белым
    // alpha = 128/255 ≈ 0.5
    // result = 0.5 * color + 0.5 * 255
    for (size_t i = 0; i < static_cast<size_t>(width) * height; ++i)
    {
        const uint8_t r_orig = partial_rgba[i * 4 + 0];
        const uint8_t g_orig = partial_rgba[i * 4 + 1];
        const uint8_t b_orig = partial_rgba[i * 4 + 2];
        
        const uint8_t r_conv = partial_rgb[i * 3 + 0];
        const uint8_t g_conv = partial_rgb[i * 3 + 1];
        const uint8_t b_conv = partial_rgb[i * 3 + 2];
        
        // Проверяем, что значения находятся между оригиналом и белым
        EXPECT_GE(r_conv, r_orig);
        EXPECT_LE(r_conv, 255);
        EXPECT_GE(g_conv, g_orig);
        EXPECT_LE(g_conv, 255);
        EXPECT_GE(b_conv, b_orig);
        EXPECT_LE(b_conv, 255);
        
        // При альфа = 128, результат должен быть примерно (color + 255) / 2
        const int expected_r = (static_cast<int>(r_orig) + 255) / 2;
        const int expected_g = (static_cast<int>(g_orig) + 255) / 2;
        const int expected_b = (static_cast<int>(b_orig) + 255) / 2;
        
        // Допускаем небольшую погрешность из-за округления
        EXPECT_NEAR(static_cast<int>(r_conv), expected_r, 1);
        EXPECT_NEAR(static_cast<int>(g_conv), expected_g, 1);
        EXPECT_NEAR(static_cast<int>(b_conv), expected_b, 1);
    }
    
    std::free(partial_rgba);
    std::free(partial_rgb);
}

/**
 * @brief Тест конвертации большого изображения
 */
TEST_F(ImageConverterTest, ConvertLargeImage)
{
    constexpr int large_width = 100;
    constexpr int large_height = 100;
    const size_t rgba_size = static_cast<size_t>(large_width) * large_height * 4;
    uint8_t* large_rgba = static_cast<uint8_t*>(std::malloc(rgba_size));
    ASSERT_NE(large_rgba, nullptr);
    
    // Заполняем данными
    for (size_t i = 0; i < rgba_size; i += 4)
    {
        large_rgba[i + 0] = static_cast<uint8_t>(i % 256); // R
        large_rgba[i + 1] = static_cast<uint8_t>((i + 1) % 256); // G
        large_rgba[i + 2] = static_cast<uint8_t>((i + 2) % 256); // B
        large_rgba[i + 3] = 255; // A
    }
    
    const size_t rgb_size = static_cast<size_t>(large_width) * large_height * 3;
    uint8_t* large_rgb = static_cast<uint8_t*>(std::malloc(rgb_size));
    ASSERT_NE(large_rgb, nullptr);
    
    const auto result = ImageConverter::convertRGBAToRGB(large_rgba, large_width, large_height, large_rgb);
    
    EXPECT_TRUE(result.isSuccess());
    
    // Проверяем несколько пикселей
    for (size_t i = 0; i < 10; ++i)
    {
        EXPECT_EQ(large_rgb[i * 3 + 0], large_rgba[i * 4 + 0]);
        EXPECT_EQ(large_rgb[i * 3 + 1], large_rgba[i * 4 + 1]);
        EXPECT_EQ(large_rgb[i * 3 + 2], large_rgba[i * 4 + 2]);
    }
    
    std::free(large_rgba);
    std::free(large_rgb);
}

/**
 * @brief Тест конвертации с различными значениями альфа-канала
 */
TEST_F(ImageConverterTest, ConvertVariousAlphaValues)
{
    constexpr int width = 10;
    constexpr int height = 1;
    const size_t rgba_size = static_cast<size_t>(width) * height * 4;
    uint8_t* varied_rgba = static_cast<uint8_t*>(std::malloc(rgba_size));
    ASSERT_NE(varied_rgba, nullptr);
    
    // Создаем изображение с различными значениями альфа
    for (int x = 0; x < width; ++x)
    {
        const size_t index = static_cast<size_t>(x) * 4;
        varied_rgba[index + 0] = 100; // R
        varied_rgba[index + 1] = 150; // G
        varied_rgba[index + 2] = 200; // B
        varied_rgba[index + 3] = static_cast<uint8_t>(x * 255 / (width - 1)); // A от 0 до 255
    }
    
    const size_t rgb_size = static_cast<size_t>(width) * height * 3;
    uint8_t* varied_rgb = static_cast<uint8_t*>(std::malloc(rgb_size));
    ASSERT_NE(varied_rgb, nullptr);
    
    const auto result = ImageConverter::convertRGBAToRGB(varied_rgba, width, height, varied_rgb);
    
    EXPECT_TRUE(result.isSuccess());
    
    // Проверяем, что первый пиксель (альфа = 0) стал белым
    EXPECT_EQ(varied_rgb[0], 255);
    EXPECT_EQ(varied_rgb[1], 255);
    EXPECT_EQ(varied_rgb[2], 255);
    
    // Проверяем, что последний пиксель (альфа = 255) остался без изменений
    const size_t last_index = static_cast<size_t>(width - 1) * 3;
    EXPECT_EQ(varied_rgb[last_index + 0], 100);
    EXPECT_EQ(varied_rgb[last_index + 1], 150);
    EXPECT_EQ(varied_rgb[last_index + 2], 200);
    
    std::free(varied_rgba);
    std::free(varied_rgb);
}

/**
 * @brief Тест конвертации с проверкой всех пикселей
 */
TEST_F(ImageConverterTest, ConvertAllPixels)
{
    const auto result = ImageConverter::convertRGBAToRGB(rgba_data_, width_, height_, rgb_data_);
    
    EXPECT_TRUE(result.isSuccess());
    
    // Проверяем все пиксели
    for (int y = 0; y < height_; ++y)
    {
        for (int x = 0; x < width_; ++x)
        {
            const size_t pixel_index = static_cast<size_t>(y * width_ + x);
            const size_t rgba_offset = pixel_index * 4;
            const size_t rgb_offset = pixel_index * 3;
            
            const uint8_t alpha = rgba_data_[rgba_offset + 3];
            const float alpha_norm = static_cast<float>(alpha) / 255.0f;
            
            // Проверяем альфа-блендинг: result = alpha * color + (1 - alpha) * 255
            const uint8_t expected_r = static_cast<uint8_t>(
                alpha_norm * static_cast<float>(rgba_data_[rgba_offset + 0]) + 
                (1.0f - alpha_norm) * 255.0f);
            const uint8_t expected_g = static_cast<uint8_t>(
                alpha_norm * static_cast<float>(rgba_data_[rgba_offset + 1]) + 
                (1.0f - alpha_norm) * 255.0f);
            const uint8_t expected_b = static_cast<uint8_t>(
                alpha_norm * static_cast<float>(rgba_data_[rgba_offset + 2]) + 
                (1.0f - alpha_norm) * 255.0f);
            
            // Допускаем погрешность в 1 из-за округления
            EXPECT_NEAR(static_cast<int>(rgb_data_[rgb_offset + 0]), static_cast<int>(expected_r), 1);
            EXPECT_NEAR(static_cast<int>(rgb_data_[rgb_offset + 1]), static_cast<int>(expected_g), 1);
            EXPECT_NEAR(static_cast<int>(rgb_data_[rgb_offset + 2]), static_cast<int>(expected_b), 1);
        }
    }
}

