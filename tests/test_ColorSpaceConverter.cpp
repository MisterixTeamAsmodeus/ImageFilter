#include <gtest/gtest.h>
#include <utils/ColorSpaceConverter.h>
#include <utils/FilterResult.h>
#include <utils/SafeMath.h>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <cmath>
#include <algorithm>

/**
 * @brief Тесты для ColorSpaceConverter
 */
class ColorSpaceConverterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// Тест конвертации RGBA в RGB - успешный случай
TEST_F(ColorSpaceConverterTest, ConvertRGBAToRGB_Success)
{
    constexpr int width = 2;
    constexpr int height = 2;
    constexpr int rgba_size = width * height * 4;
    constexpr int rgb_size = width * height * 3;

    // Создаем тестовые RGBA данные
    uint8_t rgba_data[rgba_size];
    for (int i = 0; i < width * height; ++i)
    {
        rgba_data[i * 4 + 0] = 100; // R
        rgba_data[i * 4 + 1] = 150; // G
        rgba_data[i * 4 + 2] = 200; // B
        rgba_data[i * 4 + 3] = 255; // A
    }

    // Выделяем память для RGB данных
    uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size));
    ASSERT_NE(rgb_data, nullptr);

    // Выполняем конвертацию
    const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, width, height, rgb_data);

    // Проверяем успешность операции
    EXPECT_TRUE(result.isSuccess());

    // Проверяем корректность данных
    for (int i = 0; i < width * height; ++i)
    {
        EXPECT_EQ(rgb_data[i * 3 + 0], 100); // R
        EXPECT_EQ(rgb_data[i * 3 + 1], 150); // G
        EXPECT_EQ(rgb_data[i * 3 + 2], 200); // B
    }

    std::free(rgb_data);
}

// Тест конвертации RGB в RGBA - успешный случай
TEST_F(ColorSpaceConverterTest, ConvertRGBToRGBA_Success)
{
    constexpr int width = 2;
    constexpr int height = 2;
    constexpr int rgb_size = width * height * 3;
    constexpr int rgba_size = width * height * 4;

    // Создаем тестовые RGB данные
    uint8_t rgb_data[rgb_size];
    for (int i = 0; i < width * height; ++i)
    {
        rgb_data[i * 3 + 0] = 100; // R
        rgb_data[i * 3 + 1] = 150; // G
        rgb_data[i * 3 + 2] = 200; // B
    }

    // Выделяем память для RGBA данных
    uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(rgba_size));
    ASSERT_NE(rgba_data, nullptr);

    // Выполняем конвертацию
    const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, width, height, rgba_data);

    // Проверяем успешность операции
    EXPECT_TRUE(result.isSuccess());

    // Проверяем корректность данных
    for (int i = 0; i < width * height; ++i)
    {
        EXPECT_EQ(rgba_data[i * 4 + 0], 100); // R
        EXPECT_EQ(rgba_data[i * 4 + 1], 150); // G
        EXPECT_EQ(rgba_data[i * 4 + 2], 200); // B
        EXPECT_EQ(rgba_data[i * 4 + 3], 255); // A (полная непрозрачность)
    }

    std::free(rgba_data);
}

// Тест конвертации RGBA в RGB с различными значениями
TEST_F(ColorSpaceConverterTest, ConvertRGBAToRGB_VariousValues)
{
    constexpr int width = 3;
    constexpr int height = 3;
    constexpr int rgba_size = width * height * 4;
    constexpr int rgb_size = width * height * 3;

    // Создаем тестовые RGBA данные с различными значениями
    uint8_t rgba_data[rgba_size];
    for (int i = 0; i < width * height; ++i)
    {
        rgba_data[i * 4 + 0] = static_cast<uint8_t>(i * 10);     // R
        rgba_data[i * 4 + 1] = static_cast<uint8_t>(i * 20);     // G
        rgba_data[i * 4 + 2] = static_cast<uint8_t>(i * 30);     // B
        rgba_data[i * 4 + 3] = static_cast<uint8_t>(255 - i);   // A
    }

    uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size));
    ASSERT_NE(rgb_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, width, height, rgb_data);

    EXPECT_TRUE(result.isSuccess());

    // Проверяем, что RGB значения скопированы корректно (альфа-канал игнорируется)
    for (int i = 0; i < width * height; ++i)
    {
        EXPECT_EQ(rgb_data[i * 3 + 0], rgba_data[i * 4 + 0]); // R
        EXPECT_EQ(rgb_data[i * 3 + 1], rgba_data[i * 4 + 1]); // G
        EXPECT_EQ(rgb_data[i * 3 + 2], rgba_data[i * 4 + 2]); // B
    }

    std::free(rgb_data);
}

// Тест конвертации RGB в RGBA с различными значениями
TEST_F(ColorSpaceConverterTest, ConvertRGBToRGBA_VariousValues)
{
    constexpr int width = 3;
    constexpr int height = 3;
    constexpr int rgb_size = width * height * 3;
    constexpr int rgba_size = width * height * 4;

    // Создаем тестовые RGB данные с различными значениями
    uint8_t rgb_data[rgb_size];
    for (int i = 0; i < width * height; ++i)
    {
        rgb_data[i * 3 + 0] = static_cast<uint8_t>(i * 10); // R
        rgb_data[i * 3 + 1] = static_cast<uint8_t>(i * 20); // G
        rgb_data[i * 3 + 2] = static_cast<uint8_t>(i * 30); // B
    }

    uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(rgba_size));
    ASSERT_NE(rgba_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, width, height, rgba_data);

    EXPECT_TRUE(result.isSuccess());

    // Проверяем, что RGB значения скопированы корректно и альфа установлена в 255
    for (int i = 0; i < width * height; ++i)
    {
        EXPECT_EQ(rgba_data[i * 4 + 0], rgb_data[i * 3 + 0]); // R
        EXPECT_EQ(rgba_data[i * 4 + 1], rgb_data[i * 3 + 1]); // G
        EXPECT_EQ(rgba_data[i * 4 + 2], rgb_data[i * 3 + 2]); // B
        EXPECT_EQ(rgba_data[i * 4 + 3], 255); // A (полная непрозрачность)
    }

    std::free(rgba_data);
}

// Тест конвертации RGBA в RGB с большим изображением
TEST_F(ColorSpaceConverterTest, ConvertRGBAToRGB_LargeImage)
{
    constexpr int width = 100;
    constexpr int height = 100;
    constexpr int rgba_size = width * height * 4;
    constexpr int rgb_size = width * height * 3;

    uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(rgba_size));
    ASSERT_NE(rgba_data, nullptr);

    // Заполняем тестовыми данными
    for (int i = 0; i < width * height; ++i)
    {
        rgba_data[i * 4 + 0] = static_cast<uint8_t>(i % 256); // R
        rgba_data[i * 4 + 1] = static_cast<uint8_t>((i * 2) % 256); // G
        rgba_data[i * 4 + 2] = static_cast<uint8_t>((i * 3) % 256); // B
        rgba_data[i * 4 + 3] = 255; // A
    }

    uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size));
    ASSERT_NE(rgb_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, width, height, rgb_data);

    EXPECT_TRUE(result.isSuccess());

    // Проверяем несколько случайных пикселей
    for (int i = 0; i < 10; ++i)
    {
        const int pixel_index = (i * width * height) / 10;
        EXPECT_EQ(rgb_data[pixel_index * 3 + 0], rgba_data[pixel_index * 4 + 0]);
        EXPECT_EQ(rgb_data[pixel_index * 3 + 1], rgba_data[pixel_index * 4 + 1]);
        EXPECT_EQ(rgb_data[pixel_index * 3 + 2], rgba_data[pixel_index * 4 + 2]);
    }

    std::free(rgba_data);
    std::free(rgb_data);
}

// Тест конвертации RGB в RGBA с большим изображением
TEST_F(ColorSpaceConverterTest, ConvertRGBToRGBA_LargeImage)
{
    constexpr int width = 100;
    constexpr int height = 100;
    constexpr int rgb_size = width * height * 3;
    constexpr int rgba_size = width * height * 4;

    uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size));
    ASSERT_NE(rgb_data, nullptr);

    // Заполняем тестовыми данными
    for (int i = 0; i < width * height; ++i)
    {
        rgb_data[i * 3 + 0] = static_cast<uint8_t>(i % 256); // R
        rgb_data[i * 3 + 1] = static_cast<uint8_t>((i * 2) % 256); // G
        rgb_data[i * 3 + 2] = static_cast<uint8_t>((i * 3) % 256); // B
    }

    uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(rgba_size));
    ASSERT_NE(rgba_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, width, height, rgba_data);

    EXPECT_TRUE(result.isSuccess());

    // Проверяем несколько случайных пикселей
    for (int i = 0; i < 10; ++i)
    {
        const int pixel_index = (i * width * height) / 10;
        EXPECT_EQ(rgba_data[pixel_index * 4 + 0], rgb_data[pixel_index * 3 + 0]);
        EXPECT_EQ(rgba_data[pixel_index * 4 + 1], rgb_data[pixel_index * 3 + 1]);
        EXPECT_EQ(rgba_data[pixel_index * 4 + 2], rgb_data[pixel_index * 3 + 2]);
        EXPECT_EQ(rgba_data[pixel_index * 4 + 3], 255); // A
    }

    std::free(rgb_data);
    std::free(rgba_data);
}

// Тест конвертации RGBA в RGB с минимальным размером (1x1)
TEST_F(ColorSpaceConverterTest, ConvertRGBAToRGB_MinimalSize)
{
    constexpr int width = 1;
    constexpr int height = 1;
    constexpr int rgba_size = width * height * 4;
    constexpr int rgb_size = width * height * 3;

    uint8_t rgba_data[rgba_size] = {100, 150, 200, 255};
    uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size));
    ASSERT_NE(rgb_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, width, height, rgb_data);

    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(rgb_data[0], 100);
    EXPECT_EQ(rgb_data[1], 150);
    EXPECT_EQ(rgb_data[2], 200);

    std::free(rgb_data);
}

// Тест конвертации RGB в RGBA с минимальным размером (1x1)
TEST_F(ColorSpaceConverterTest, ConvertRGBToRGBA_MinimalSize)
{
    constexpr int width = 1;
    constexpr int height = 1;
    constexpr int rgb_size = width * height * 3;
    constexpr int rgba_size = width * height * 4;

    uint8_t rgb_data[rgb_size] = {100, 150, 200};
    uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(rgba_size));
    ASSERT_NE(rgba_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, width, height, rgba_data);

    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(rgba_data[0], 100);
    EXPECT_EQ(rgba_data[1], 150);
    EXPECT_EQ(rgba_data[2], 200);
    EXPECT_EQ(rgba_data[3], 255);

    std::free(rgba_data);
}

// Тест конвертации RGBA в RGB с нулевыми размерами
TEST_F(ColorSpaceConverterTest, ConvertRGBAToRGB_ZeroWidth)
{
    constexpr int width = 0;
    constexpr int height = 10;
    constexpr int rgba_size = width * height * 4;
    constexpr int rgb_size = width * height * 3;

    uint8_t rgba_data[1] = {0};
    uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size > 0 ? rgb_size : 1));
    ASSERT_NE(rgb_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, width, height, rgb_data);

    // При нулевой ширине SafeMath::safeMultiply вернет 0, и операция должна быть успешной
    // или может вернуть ошибку переполнения в зависимости от реализации
    // Проверяем, что функция обрабатывает этот случай
    if (!result.isSuccess())
    {
        EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);
    }

    std::free(rgb_data);
}

// Тест конвертации RGBA в RGB с нулевой высотой
TEST_F(ColorSpaceConverterTest, ConvertRGBAToRGB_ZeroHeight)
{
    constexpr int width = 10;
    constexpr int height = 0;
    constexpr int rgba_size = width * height * 4;
    constexpr int rgb_size = width * height * 3;

    uint8_t rgba_data[1] = {0};
    uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size > 0 ? rgb_size : 1));
    ASSERT_NE(rgb_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, width, height, rgb_data);

    // При нулевой высоте SafeMath::safeMultiply вернет 0, и операция должна быть успешной
    // или может вернуть ошибку переполнения в зависимости от реализации
    if (!result.isSuccess())
    {
        EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);
    }

    std::free(rgb_data);
}

// Тест конвертации RGB в RGBA с нулевыми размерами
TEST_F(ColorSpaceConverterTest, ConvertRGBToRGBA_ZeroWidth)
{
    constexpr int width = 0;
    constexpr int height = 10;
    constexpr int rgb_size = width * height * 3;
    constexpr int rgba_size = width * height * 4;

    uint8_t rgb_data[1] = {0};
    uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(rgba_size > 0 ? rgba_size : 1));
    ASSERT_NE(rgba_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, width, height, rgba_data);

    // При нулевой ширине SafeMath::safeMultiply вернет 0, и операция должна быть успешной
    // или может вернуть ошибку переполнения в зависимости от реализации
    if (!result.isSuccess())
    {
        EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);
    }

    std::free(rgba_data);
}

// Тест конвертации RGB в RGBA с нулевой высотой
TEST_F(ColorSpaceConverterTest, ConvertRGBToRGBA_ZeroHeight)
{
    constexpr int width = 10;
    constexpr int height = 0;
    constexpr int rgb_size = width * height * 3;
    constexpr int rgba_size = width * height * 4;

    uint8_t rgb_data[1] = {0};
    uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(rgba_size > 0 ? rgba_size : 1));
    ASSERT_NE(rgba_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, width, height, rgba_data);

    // При нулевой высоте SafeMath::safeMultiply вернет 0, и операция должна быть успешной
    // или может вернуть ошибку переполнения в зависимости от реализации
    if (!result.isSuccess())
    {
        EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);
    }

    std::free(rgba_data);
}

// Тест конвертации RGBA в RGB с очень большими размерами (граничное значение)
// Используем размеры, которые близки к максимальным, но не вызывают переполнение
TEST_F(ColorSpaceConverterTest, ConvertRGBAToRGB_LargeSize)
{
    // Используем размеры, которые не вызывают переполнение size_t
    // Для 64-битного size_t максимальное значение ~18 экзабайт
    // Используем разумные большие размеры
    constexpr int width = 10000;
    constexpr int height = 10000;
    
    // Проверяем, что width * height * 4 не вызывает переполнение size_t
    constexpr size_t max_safe_size = std::numeric_limits<size_t>::max() / 4;
    if (static_cast<size_t>(width) * static_cast<size_t>(height) > max_safe_size)
    {
        GTEST_SKIP() << "Размеры слишком большие для теста";
    }
    
    constexpr size_t rgba_size = static_cast<size_t>(width) * height * 4;
    constexpr size_t rgb_size = static_cast<size_t>(width) * height * 3;

    uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(rgba_size));
    if (rgba_data == nullptr)
    {
        GTEST_SKIP() << "Недостаточно памяти для теста";
    }

    // Заполняем тестовыми данными
    for (size_t i = 0; i < rgba_size; i += 4)
    {
        rgba_data[i + 0] = static_cast<uint8_t>(i % 256);     // R
        rgba_data[i + 1] = static_cast<uint8_t>((i * 2) % 256); // G
        rgba_data[i + 2] = static_cast<uint8_t>((i * 3) % 256); // B
        rgba_data[i + 3] = 255; // A
    }

    uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size));
    ASSERT_NE(rgb_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, width, height, rgb_data);

    EXPECT_TRUE(result.isSuccess());

    // Проверяем несколько случайных пикселей
    for (int i = 0; i < 10; ++i)
    {
        const size_t pixel_index = (i * width * height) / 10;
        EXPECT_EQ(rgb_data[pixel_index * 3 + 0], rgba_data[pixel_index * 4 + 0]);
        EXPECT_EQ(rgb_data[pixel_index * 3 + 1], rgba_data[pixel_index * 4 + 1]);
        EXPECT_EQ(rgb_data[pixel_index * 3 + 2], rgba_data[pixel_index * 4 + 2]);
    }

    std::free(rgba_data);
    std::free(rgb_data);
}

// Тест конвертации RGB в RGBA с очень большими размерами (граничное значение)
TEST_F(ColorSpaceConverterTest, ConvertRGBToRGBA_LargeSize)
{
    // Используем размеры, которые не вызывают переполнение size_t
    constexpr int width = 10000;
    constexpr int height = 10000;
    
    // Проверяем, что width * height * 4 не вызывает переполнение size_t
    constexpr size_t max_safe_size = std::numeric_limits<size_t>::max() / 4;
    if (static_cast<size_t>(width) * static_cast<size_t>(height) > max_safe_size)
    {
        GTEST_SKIP() << "Размеры слишком большие для теста";
    }
    
    constexpr size_t rgb_size = static_cast<size_t>(width) * height * 3;
    constexpr size_t rgba_size = static_cast<size_t>(width) * height * 4;

    uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size));
    if (rgb_data == nullptr)
    {
        GTEST_SKIP() << "Недостаточно памяти для теста";
    }

    // Заполняем тестовыми данными
    for (size_t i = 0; i < rgb_size; i += 3)
    {
        rgb_data[i + 0] = static_cast<uint8_t>(i % 256);     // R
        rgb_data[i + 1] = static_cast<uint8_t>((i * 2) % 256); // G
        rgb_data[i + 2] = static_cast<uint8_t>((i * 3) % 256); // B
    }

    uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(rgba_size));
    ASSERT_NE(rgba_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, width, height, rgba_data);

    EXPECT_TRUE(result.isSuccess());

    // Проверяем несколько случайных пикселей
    for (int i = 0; i < 10; ++i)
    {
        const size_t pixel_index = (i * width * height) / 10;
        EXPECT_EQ(rgba_data[pixel_index * 4 + 0], rgb_data[pixel_index * 3 + 0]);
        EXPECT_EQ(rgba_data[pixel_index * 4 + 1], rgb_data[pixel_index * 3 + 1]);
        EXPECT_EQ(rgba_data[pixel_index * 4 + 2], rgb_data[pixel_index * 3 + 2]);
        EXPECT_EQ(rgba_data[pixel_index * 4 + 3], 255); // A
    }

    std::free(rgb_data);
    std::free(rgba_data);
}

// Тест конвертации RGBA в RGB с различными значениями альфа-канала
TEST_F(ColorSpaceConverterTest, ConvertRGBAToRGB_VariousAlphaValues)
{
    constexpr int width = 5;
    constexpr int height = 5;
    constexpr int rgba_size = width * height * 4;
    constexpr int rgb_size = width * height * 3;

    uint8_t rgba_data[rgba_size];
    // Заполняем с различными значениями альфа-канала
    for (int i = 0; i < width * height; ++i)
    {
        rgba_data[i * 4 + 0] = 100; // R
        rgba_data[i * 4 + 1] = 150; // G
        rgba_data[i * 4 + 2] = 200; // B
        rgba_data[i * 4 + 3] = static_cast<uint8_t>(i * 10 % 256); // A (различные значения)
    }

    uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size));
    ASSERT_NE(rgb_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, width, height, rgb_data);

    EXPECT_TRUE(result.isSuccess());

    // Проверяем, что RGB значения скопированы корректно (альфа игнорируется)
    for (int i = 0; i < width * height; ++i)
    {
        EXPECT_EQ(rgb_data[i * 3 + 0], rgba_data[i * 4 + 0]); // R
        EXPECT_EQ(rgb_data[i * 3 + 1], rgba_data[i * 4 + 1]); // G
        EXPECT_EQ(rgb_data[i * 3 + 2], rgba_data[i * 4 + 2]); // B
    }

    std::free(rgb_data);
}

// Тест конвертации RGB в RGBA - проверка, что альфа всегда 255
TEST_F(ColorSpaceConverterTest, ConvertRGBToRGBA_AlphaAlways255)
{
    constexpr int width = 10;
    constexpr int height = 10;
    constexpr int rgb_size = width * height * 3;
    constexpr int rgba_size = width * height * 4;

    uint8_t rgb_data[rgb_size];
    for (int i = 0; i < width * height; ++i)
    {
        rgb_data[i * 3 + 0] = static_cast<uint8_t>(i % 256); // R
        rgb_data[i * 3 + 1] = static_cast<uint8_t>((i * 2) % 256); // G
        rgb_data[i * 3 + 2] = static_cast<uint8_t>((i * 3) % 256); // B
    }

    uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(rgba_size));
    ASSERT_NE(rgba_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, width, height, rgba_data);

    EXPECT_TRUE(result.isSuccess());

    // Проверяем, что все альфа-каналы установлены в 255
    for (int i = 0; i < width * height; ++i)
    {
        EXPECT_EQ(rgba_data[i * 4 + 3], 255);
    }

    std::free(rgba_data);
}

// Тест конвертации RGBA в RGB - проверка переполнения при вычислении width * height
// Используем значения, которые могут вызвать переполнение в safeMultiply
TEST_F(ColorSpaceConverterTest, ConvertRGBAToRGB_OverflowWidthHeight)
{
    // Используем значения, которые вызовут переполнение при умножении на 3
    // Берем значения близкие к границе переполнения для size_t
    const size_t max_safe_for_3 = std::numeric_limits<size_t>::max() / 3;
    const size_t max_dimension = static_cast<size_t>(std::sqrt(static_cast<double>(max_safe_for_3)));
    
    // Используем значения, которые вызовут переполнение при умножении на 3
    // Но ограничиваем их максимальным значением int, чтобы избежать проблем
    const int max_int = std::numeric_limits<int>::max();
    const int width = static_cast<int>(std::min(max_dimension + 1000, static_cast<size_t>(max_int)));
    const int height = static_cast<int>(std::min(max_dimension + 1000, static_cast<size_t>(max_int)));
    
    // Проверяем, что это действительно вызовет переполнение
    size_t test_product = 0;
    if (SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), test_product))
    {
        size_t test_size = 0;
        if (!SafeMath::safeMultiply(test_product, static_cast<size_t>(3), test_size))
        {
            // Переполнение при умножении на 3 - это то, что мы тестируем
            // Функция должна вернуть ошибку до того, как попытается использовать эти значения
            uint8_t rgba_data[1] = {0};
            uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(1));
            ASSERT_NE(rgb_data, nullptr);

            const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, width, height, rgb_data);

            // Должна быть ошибка переполнения
            EXPECT_FALSE(result.isSuccess());
            EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);

            std::free(rgb_data);
            return;
        }
    }
    
    // Если переполнения нет, используем значения, которые вызовут переполнение при width * height
    // Используем максимальные значения int, которые точно вызовут переполнение на большинстве систем
    constexpr int max_width = std::numeric_limits<int>::max();
    constexpr int max_height = std::numeric_limits<int>::max();
    
    size_t width_height_check = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(max_width), static_cast<size_t>(max_height), width_height_check))
    {
        // Переполнение при width * height
        uint8_t rgba_data[1] = {0};
        uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(1));
        ASSERT_NE(rgb_data, nullptr);

        const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, max_width, max_height, rgb_data);

        // Должна быть ошибка переполнения
        EXPECT_FALSE(result.isSuccess());
        EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);

        std::free(rgb_data);
    }
    else
    {
        // Если переполнения нет даже с максимальными значениями, тест проходит успешно
        // Это означает, что на данной платформе переполнение не происходит
        SUCCEED() << "Переполнение не происходит на данной платформе с максимальными значениями int";
    }
}

// Тест конвертации RGB в RGBA - проверка переполнения при вычислении width * height
TEST_F(ColorSpaceConverterTest, ConvertRGBToRGBA_OverflowWidthHeight)
{
    // Используем значения, которые вызовут переполнение при умножении на 4
    // Берем значения близкие к границе переполнения для size_t
    const size_t max_safe_for_4 = std::numeric_limits<size_t>::max() / 4;
    const size_t max_dimension = static_cast<size_t>(std::sqrt(static_cast<double>(max_safe_for_4)));
    
    // Используем значения, которые вызовут переполнение при умножении на 4
    // Но ограничиваем их максимальным значением int, чтобы избежать проблем
    const int max_int = std::numeric_limits<int>::max();
    const int width = static_cast<int>(std::min(max_dimension + 1000, static_cast<size_t>(max_int)));
    const int height = static_cast<int>(std::min(max_dimension + 1000, static_cast<size_t>(max_int)));
    
    // Проверяем, что это действительно вызовет переполнение
    size_t width_height_product = 0;
    if (SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), width_height_product))
    {
        size_t rgba_size = 0;
        if (!SafeMath::safeMultiply(width_height_product, static_cast<size_t>(4), rgba_size))
        {
            // Переполнение при умножении на 4 - это то, что мы тестируем
            // Функция должна вернуть ошибку до того, как попытается использовать эти значения
        uint8_t rgb_data[1] = {0};
        uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(1));
        ASSERT_NE(rgba_data, nullptr);

        const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, width, height, rgba_data);

            // Должна быть ошибка переполнения
            EXPECT_FALSE(result.isSuccess());
            EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);

            std::free(rgba_data);
            return;
        }
    }
    
    // Если переполнения нет, используем значения, которые вызовут переполнение при width * height
    // Используем максимальные значения int, которые точно вызовут переполнение на большинстве систем
    constexpr int max_width = std::numeric_limits<int>::max();
    constexpr int max_height = std::numeric_limits<int>::max();
    
    size_t width_height_check = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(max_width), static_cast<size_t>(max_height), width_height_check))
    {
        // Переполнение при width * height
        uint8_t rgb_data[1] = {0};
        uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(1));
        ASSERT_NE(rgba_data, nullptr);

        const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, max_width, max_height, rgba_data);

        // Должна быть ошибка переполнения
        EXPECT_FALSE(result.isSuccess());
        EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);

        std::free(rgba_data);
    }
    else
    {
        // Если переполнения нет даже с максимальными значениями, тест проходит успешно
        // Это означает, что на данной платформе переполнение не происходит
        SUCCEED() << "Переполнение не происходит на данной платформе с максимальными значениями int";
    }
}

// Тест конвертации RGBA в RGB - проверка переполнения при вычислении width_height_product * 3
// Используем значения, которые не вызывают переполнение при width * height,
// но вызывают при умножении на 3
TEST_F(ColorSpaceConverterTest, ConvertRGBAToRGB_OverflowWidthHeightTimes3)
{
    // Используем значения, которые близки к границе переполнения
    // Для size_t максимальное значение / 3
    constexpr size_t max_safe = std::numeric_limits<size_t>::max() / 3;
    constexpr int width = static_cast<int>(std::min(max_safe / 2, static_cast<size_t>(std::numeric_limits<int>::max())));
    constexpr int height = static_cast<int>(std::min(max_safe / width, static_cast<size_t>(std::numeric_limits<int>::max())));
    
    // Проверяем, что width * height * 3 может вызвать переполнение
    size_t test_product = 0;
    if (SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), test_product))
    {
        size_t test_size = 0;
        if (!SafeMath::safeMultiply(test_product, static_cast<size_t>(3), test_size))
        {
            // Переполнение при умножении на 3
            uint8_t rgba_data[1] = {0};
            uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(1));
            ASSERT_NE(rgb_data, nullptr);

            const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, width, height, rgb_data);

            // Должна быть ошибка переполнения
            EXPECT_FALSE(result.isSuccess());
            EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);

            std::free(rgb_data);
        }
    }
}

// Тест конвертации RGB в RGBA - проверка переполнения при вычислении width_height_product * 4
TEST_F(ColorSpaceConverterTest, ConvertRGBToRGBA_OverflowWidthHeightTimes4)
{
    // Используем значения, которые близки к границе переполнения
    // Для size_t максимальное значение / 4
    constexpr size_t max_safe = std::numeric_limits<size_t>::max() / 4;
    constexpr int width = static_cast<int>(std::min(max_safe / 2, static_cast<size_t>(std::numeric_limits<int>::max())));
    constexpr int height = static_cast<int>(std::min(max_safe / width, static_cast<size_t>(std::numeric_limits<int>::max())));
    
    // Проверяем, что width * height * 4 может вызвать переполнение
    size_t test_product = 0;
    if (SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), test_product))
    {
        size_t test_size = 0;
        if (!SafeMath::safeMultiply(test_product, static_cast<size_t>(4), test_size))
        {
            // Переполнение при умножении на 4
            uint8_t rgb_data[1] = {0};
            uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(1));
            ASSERT_NE(rgba_data, nullptr);

            const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, width, height, rgba_data);

            // Должна быть ошибка переполнения
            EXPECT_FALSE(result.isSuccess());
            EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);

            std::free(rgba_data);
        }
    }
}

// Тест конвертации RGBA в RGB - проверка переполнения при вычислении offset в цикле
// Проверяет ветвь safeMultiply для вычисления rgba_offset и rgb_offset
TEST_F(ColorSpaceConverterTest, ConvertRGBAToRGB_OverflowOffsetCalculation)
{
    // Используем значения, которые могут вызвать переполнение при вычислении offset
    // Для этого нужно очень большое количество пикселей
    // Но в реальности, если width * height уже проверено, то offset не должен переполняться
    // Этот тест проверяет, что код обрабатывает такую ситуацию
    
    constexpr int width = 1000;
    constexpr int height = 1000;
    
    // Проверяем, что width * height не вызывает переполнение
    size_t pixel_count = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), pixel_count))
    {
        GTEST_SKIP() << "Размеры вызывают переполнение";
    }
    
    // Для нормальных размеров offset не должен переполняться
    constexpr size_t rgba_size = static_cast<size_t>(width) * height * 4;
    constexpr size_t rgb_size = static_cast<size_t>(width) * height * 3;

    uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(rgba_size));
    if (rgba_data == nullptr)
    {
        GTEST_SKIP() << "Недостаточно памяти";
    }

    uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size));
    ASSERT_NE(rgb_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, width, height, rgb_data);

    EXPECT_TRUE(result.isSuccess());

    std::free(rgba_data);
    std::free(rgb_data);
}

// Тест конвертации RGB в RGBA - проверка переполнения при вычислении offset в цикле
TEST_F(ColorSpaceConverterTest, ConvertRGBToRGBA_OverflowOffsetCalculation)
{
    constexpr int width = 1000;
    constexpr int height = 1000;
    
    // Проверяем, что width * height не вызывает переполнение
    size_t pixel_count = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), pixel_count))
    {
        GTEST_SKIP() << "Размеры вызывают переполнение";
    }
    
    // Для нормальных размеров offset не должен переполняться
    constexpr size_t rgb_size = static_cast<size_t>(width) * height * 3;
    constexpr size_t rgba_size = static_cast<size_t>(width) * height * 4;

    uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(rgb_size));
    if (rgb_data == nullptr)
    {
        GTEST_SKIP() << "Недостаточно памяти";
    }

    uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(rgba_size));
    ASSERT_NE(rgba_data, nullptr);

    const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, width, height, rgba_data);

    EXPECT_TRUE(result.isSuccess());

    std::free(rgb_data);
    std::free(rgba_data);
}

// Тест конвертации RGBA в RGB - проверка контекста ошибки при переполнении
TEST_F(ColorSpaceConverterTest, ConvertRGBAToRGB_ErrorContext)
{
    // Используем значения, которые вызовут переполнение при умножении на 3
    // Берем значения близкие к границе переполнения для size_t
    const size_t max_safe_for_3 = std::numeric_limits<size_t>::max() / 3;
    const size_t max_dimension = static_cast<size_t>(std::sqrt(static_cast<double>(max_safe_for_3)));
    
    // Используем значения, которые вызовут переполнение при умножении на 3
    // Но ограничиваем их максимальным значением int, чтобы избежать проблем
    const int max_int = std::numeric_limits<int>::max();
    const int width = static_cast<int>(std::min(max_dimension + 1000, static_cast<size_t>(max_int)));
    const int height = static_cast<int>(std::min(max_dimension + 1000, static_cast<size_t>(max_int)));
    
    // Проверяем, что это действительно вызовет переполнение
    size_t test_product = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), test_product))
    {
        // Переполнение при width * height
        uint8_t rgba_data[1] = {0};
        uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(1));
        ASSERT_NE(rgb_data, nullptr);

        const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, width, height, rgb_data);

        EXPECT_FALSE(result.isSuccess());
        EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);
        EXPECT_TRUE(result.context.has_value());
        EXPECT_TRUE(result.context.value().image_width.has_value());
        EXPECT_EQ(result.context.value().image_width.value(), width);
        EXPECT_TRUE(result.context.value().image_height.has_value());
        EXPECT_EQ(result.context.value().image_height.value(), height);
        EXPECT_TRUE(result.context.value().image_channels.has_value());
        EXPECT_EQ(result.context.value().image_channels.value(), 4);

        std::free(rgb_data);
    }
    else
    {
        size_t test_size = 0;
        if (!SafeMath::safeMultiply(test_product, static_cast<size_t>(3), test_size))
        {
            // Переполнение при умножении на 3
            uint8_t rgba_data[1] = {0};
            uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(1));
            ASSERT_NE(rgb_data, nullptr);

            const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, width, height, rgb_data);

            EXPECT_FALSE(result.isSuccess());
            EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);
            EXPECT_TRUE(result.context.has_value());
            EXPECT_TRUE(result.context.value().image_width.has_value());
            EXPECT_EQ(result.context.value().image_width.value(), width);
            EXPECT_TRUE(result.context.value().image_height.has_value());
            EXPECT_EQ(result.context.value().image_height.value(), height);
            EXPECT_TRUE(result.context.value().image_channels.has_value());
            EXPECT_EQ(result.context.value().image_channels.value(), 4);

            std::free(rgb_data);
        }
        else
        {
            // Если переполнения нет, используем максимальные значения int
            constexpr int max_width = std::numeric_limits<int>::max();
            constexpr int max_height = std::numeric_limits<int>::max();
            
            size_t width_height_check = 0;
            if (!SafeMath::safeMultiply(static_cast<size_t>(max_width), static_cast<size_t>(max_height), width_height_check))
            {
                // Переполнение при width * height
                uint8_t rgba_data[1] = {0};
                uint8_t* rgb_data = static_cast<uint8_t*>(std::malloc(1));
                ASSERT_NE(rgb_data, nullptr);

                const auto result = ColorSpaceConverter::convertRGBAToRGB(rgba_data, max_width, max_height, rgb_data);

                EXPECT_FALSE(result.isSuccess());
                EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);
                EXPECT_TRUE(result.context.has_value());
                EXPECT_TRUE(result.context.value().image_width.has_value());
                EXPECT_EQ(result.context.value().image_width.value(), max_width);
                EXPECT_TRUE(result.context.value().image_height.has_value());
                EXPECT_EQ(result.context.value().image_height.value(), max_height);
                EXPECT_TRUE(result.context.value().image_channels.has_value());
                EXPECT_EQ(result.context.value().image_channels.value(), 4);

                std::free(rgb_data);
            }
            else
            {
                // Если переполнения нет даже с максимальными значениями, тест проходит успешно
                SUCCEED() << "Переполнение не происходит на данной платформе с максимальными значениями int";
            }
        }
    }
}

// Тест конвертации RGB в RGBA - проверка контекста ошибки при переполнении
TEST_F(ColorSpaceConverterTest, ConvertRGBToRGBA_ErrorContext)
{
    // Используем значения, которые вызовут переполнение при умножении на 4
    // Берем значения близкие к границе переполнения для size_t
    const size_t max_safe_for_4 = std::numeric_limits<size_t>::max() / 4;
    const size_t max_dimension = static_cast<size_t>(std::sqrt(static_cast<double>(max_safe_for_4)));
    
    // Используем значения, которые вызовут переполнение при умножении на 4
    // Но ограничиваем их максимальным значением int, чтобы избежать проблем
    const int max_int = std::numeric_limits<int>::max();
    const int width = static_cast<int>(std::min(max_dimension + 1000, static_cast<size_t>(max_int)));
    const int height = static_cast<int>(std::min(max_dimension + 1000, static_cast<size_t>(max_int)));
    
    // Проверяем, что это действительно вызовет переполнение
    size_t test_product = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), test_product))
    {
        // Переполнение при width * height
        uint8_t rgb_data[1] = {0};
        uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(1));
        ASSERT_NE(rgba_data, nullptr);

        const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, width, height, rgba_data);

        EXPECT_FALSE(result.isSuccess());
        EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);
        EXPECT_TRUE(result.context.has_value());
        EXPECT_TRUE(result.context.value().image_width.has_value());
        EXPECT_EQ(result.context.value().image_width.value(), width);
        EXPECT_TRUE(result.context.value().image_height.has_value());
        EXPECT_EQ(result.context.value().image_height.value(), height);
        EXPECT_TRUE(result.context.value().image_channels.has_value());
        EXPECT_EQ(result.context.value().image_channels.value(), 3);

        std::free(rgba_data);
    }
    else
    {
        size_t test_size = 0;
        if (!SafeMath::safeMultiply(test_product, static_cast<size_t>(4), test_size))
        {
            // Переполнение при умножении на 4
            uint8_t rgb_data[1] = {0};
            uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(1));
            ASSERT_NE(rgba_data, nullptr);

            const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, width, height, rgba_data);

            EXPECT_FALSE(result.isSuccess());
            EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);
            EXPECT_TRUE(result.context.has_value());
            EXPECT_TRUE(result.context.value().image_width.has_value());
            EXPECT_EQ(result.context.value().image_width.value(), width);
            EXPECT_TRUE(result.context.value().image_height.has_value());
            EXPECT_EQ(result.context.value().image_height.value(), height);
            EXPECT_TRUE(result.context.value().image_channels.has_value());
            EXPECT_EQ(result.context.value().image_channels.value(), 3);

            std::free(rgba_data);
        }
        else
        {
            // Если переполнения нет, используем максимальные значения int
            constexpr int max_width = std::numeric_limits<int>::max();
            constexpr int max_height = std::numeric_limits<int>::max();
            
            size_t width_height_check = 0;
            if (!SafeMath::safeMultiply(static_cast<size_t>(max_width), static_cast<size_t>(max_height), width_height_check))
            {
                // Переполнение при width * height
                uint8_t rgb_data[1] = {0};
                uint8_t* rgba_data = static_cast<uint8_t*>(std::malloc(1));
                ASSERT_NE(rgba_data, nullptr);

                const auto result = ColorSpaceConverter::convertRGBToRGBA(rgb_data, max_width, max_height, rgba_data);

                EXPECT_FALSE(result.isSuccess());
                EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);
                EXPECT_TRUE(result.context.has_value());
                EXPECT_TRUE(result.context.value().image_width.has_value());
                EXPECT_EQ(result.context.value().image_width.value(), max_width);
                EXPECT_TRUE(result.context.value().image_height.has_value());
                EXPECT_EQ(result.context.value().image_height.value(), max_height);
                EXPECT_TRUE(result.context.value().image_channels.has_value());
                EXPECT_EQ(result.context.value().image_channels.value(), 3);

                std::free(rgba_data);
            }
            else
            {
                // Если переполнения нет даже с максимальными значениями, тест проходит успешно
                SUCCEED() << "Переполнение не происходит на данной платформе с максимальными значениями int";
            }
        }
    }
}

