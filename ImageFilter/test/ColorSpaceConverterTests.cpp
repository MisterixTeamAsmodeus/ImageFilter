/**
 * @file ColorSpaceConverterTests.cpp
 * @brief Юнит-тесты для конвертации между цветовыми пространствами RGB и RGBA.
 *
 * Покрываются базовые сценарии корректного копирования компонент
 * и сохранения значения альфа-канала по умолчанию.
 */

#include <gtest/gtest.h>

#include <utils/ColorSpaceConverter.h>

/**
 * @brief Проверка конвертации из RGBA в RGB на небольшом изображении 2x1.
 */
TEST(ColorSpaceConverterTests, ConvertRGBAToRGB_Basic)
{
    const int width = 2;
    const int height = 1;

    const uint8_t rgba_data[] = {
        10, 20, 30, 255,  // первый пиксель
        40, 50, 60, 128   // второй пиксель
    };

    uint8_t rgb_data[width * height * 3] = {};

    const auto result = ColorSpaceConverter::convertRGBAToRGB(
        rgba_data,
        width,
        height,
        rgb_data);

    ASSERT_TRUE(result.isSuccess());

    EXPECT_EQ(rgb_data[0], 10);
    EXPECT_EQ(rgb_data[1], 20);
    EXPECT_EQ(rgb_data[2], 30);
    EXPECT_EQ(rgb_data[3], 40);
    EXPECT_EQ(rgb_data[4], 50);
    EXPECT_EQ(rgb_data[5], 60);
}

/**
 * @brief Проверка конвертации из RGB в RGBA и установки альфа-канала в 255.
 */
TEST(ColorSpaceConverterTests, ConvertRGBToRGBA_Basic)
{
    const int width = 2;
    const int height = 1;

    const uint8_t rgb_data[] = {
        5, 15, 25,
        35, 45, 55
    };

    uint8_t rgba_data[width * height * 4] = {};

    const auto result = ColorSpaceConverter::convertRGBToRGBA(
        rgb_data,
        width,
        height,
        rgba_data);

    ASSERT_TRUE(result.isSuccess());

    EXPECT_EQ(rgba_data[0], 5);
    EXPECT_EQ(rgba_data[1], 15);
    EXPECT_EQ(rgba_data[2], 25);
    EXPECT_EQ(rgba_data[3], 255);

    EXPECT_EQ(rgba_data[4], 35);
    EXPECT_EQ(rgba_data[5], 45);
    EXPECT_EQ(rgba_data[6], 55);
    EXPECT_EQ(rgba_data[7], 255);
}


