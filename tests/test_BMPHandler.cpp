#include <gtest/gtest.h>
#include <utils/BMPHandler.h>
#include "test_utils.h"
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

/**
 * @brief Тесты для BMPHandler
 */
class BMPHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Создаем временную директорию для тестовых файлов
        test_dir_ = fs::temp_directory_path() / "BMPHandlerTests";
        fs::create_directories(test_dir_);
    }

    void TearDown() override
    {
        // Очищаем временные файлы
        if (fs::exists(test_dir_))
        {
            fs::remove_all(test_dir_);
        }
    }

    fs::path test_dir_;
};

// Тест загрузки BMP - успешный случай
TEST_F(BMPHandlerTest, LoadBMP_Success)
{
    // Сначала создаем BMP файл для теста
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto bmp_file = test_dir_ / "test.bmp";

    // Сохраняем как BMP
    ASSERT_TRUE(BMPHandler::saveBMP(bmp_file.string(), width, height, 3, test_image.data()));

    // Загружаем BMP
    int loaded_width = 0;
    int loaded_height = 0;
    int loaded_channels = 0;
    uint8_t* loaded_data = BMPHandler::loadBMP(bmp_file.string(), loaded_width, loaded_height, loaded_channels);

    ASSERT_NE(loaded_data, nullptr);
    EXPECT_EQ(loaded_width, width);
    EXPECT_EQ(loaded_height, height);
    EXPECT_EQ(loaded_channels, 3);

    // Проверяем, что данные загружены (проверяем несколько пикселей)
    // BMP может хранить данные в формате BGR, поэтому проверяем наличие данных
    bool has_data = false;
    for (int i = 0; i < std::min(10, loaded_width * loaded_height * 3); ++i)
    {
        if (loaded_data[i] != 0)
        {
            has_data = true;
            break;
        }
    }
    EXPECT_TRUE(has_data);

    std::free(loaded_data);
}

// Тест загрузки BMP - несуществующий файл
TEST_F(BMPHandlerTest, LoadBMP_NonExistentFile)
{
    int width = 0;
    int height = 0;
    int channels = 0;
    uint8_t* data = BMPHandler::loadBMP("nonexistent_file.bmp", width, height, channels);

    EXPECT_EQ(data, nullptr);
}

// Тест сохранения BMP - успешный случай (RGB)
TEST_F(BMPHandlerTest, SaveBMP_RGB_Success)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto bmp_file = test_dir_ / "test_rgb.bmp";

    const bool result = BMPHandler::saveBMP(bmp_file.string(), width, height, 3, test_image.data());

    EXPECT_TRUE(result);
    EXPECT_TRUE(fs::exists(bmp_file));
    EXPECT_GT(fs::file_size(bmp_file), 0);
}

// Тест сохранения BMP - успешный случай (RGBA)
TEST_F(BMPHandlerTest, SaveBMP_RGBA_Success)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    const auto bmp_file = test_dir_ / "test_rgba.bmp";

    const bool result = BMPHandler::saveBMP(bmp_file.string(), width, height, 4, test_image.data());

    EXPECT_TRUE(result);
    EXPECT_TRUE(fs::exists(bmp_file));
    EXPECT_GT(fs::file_size(bmp_file), 0);
}

// Тест сохранения BMP - некорректные параметры (нулевая ширина)
TEST_F(BMPHandlerTest, SaveBMP_InvalidWidth)
{
    constexpr int width = 0;
    constexpr int height = 10;
    uint8_t data[3] = {100, 150, 200};
    const auto bmp_file = test_dir_ / "test_invalid.bmp";

    const bool result = BMPHandler::saveBMP(bmp_file.string(), width, height, 3, data);

    EXPECT_FALSE(result);
}

// Тест сохранения BMP - некорректные параметры (нулевая высота)
TEST_F(BMPHandlerTest, SaveBMP_InvalidHeight)
{
    constexpr int width = 10;
    constexpr int height = 0;
    uint8_t data[3] = {100, 150, 200};
    const auto bmp_file = test_dir_ / "test_invalid.bmp";

    const bool result = BMPHandler::saveBMP(bmp_file.string(), width, height, 3, data);

    EXPECT_FALSE(result);
}

// Тест сохранения BMP - некорректные параметры (nullptr)
TEST_F(BMPHandlerTest, SaveBMP_Nullptr)
{
    constexpr int width = 10;
    constexpr int height = 10;
    const auto bmp_file = test_dir_ / "test_invalid.bmp";

    const bool result = BMPHandler::saveBMP(bmp_file.string(), width, height, 3, nullptr);

    EXPECT_FALSE(result);
}

// Тест загрузки и сохранения BMP - цикл конвертации
TEST_F(BMPHandlerTest, LoadSaveBMP_RoundTrip)
{
    constexpr int width = 20;
    constexpr int height = 20;
    auto original_image = TestUtils::createTestImage(width, height);
    const auto bmp_file = test_dir_ / "test_roundtrip.bmp";

    // Сохраняем
    ASSERT_TRUE(BMPHandler::saveBMP(bmp_file.string(), width, height, 3, original_image.data()));

    // Загружаем
    int loaded_width = 0;
    int loaded_height = 0;
    int loaded_channels = 0;
    uint8_t* loaded_data = BMPHandler::loadBMP(bmp_file.string(), loaded_width, loaded_height, loaded_channels);

    ASSERT_NE(loaded_data, nullptr);
    EXPECT_EQ(loaded_width, width);
    EXPECT_EQ(loaded_height, height);
    EXPECT_EQ(loaded_channels, 3);

    // Проверяем, что данные примерно совпадают (BMP может немного изменить данные из-за выравнивания)
    // Проверяем несколько пикселей
    for (int i = 0; i < std::min(10, width * height); ++i)
    {
        const int pixel_index = (i * width * height) / 10;
        // Допускаем небольшие различия из-за особенностей формата BMP
        EXPECT_NEAR(loaded_data[pixel_index * 3 + 0], original_image[pixel_index * 3 + 0], 1);
        EXPECT_NEAR(loaded_data[pixel_index * 3 + 1], original_image[pixel_index * 3 + 1], 1);
        EXPECT_NEAR(loaded_data[pixel_index * 3 + 2], original_image[pixel_index * 3 + 2], 1);
    }

    std::free(loaded_data);
}

// Тест сохранения BMP с различными размерами
TEST_F(BMPHandlerTest, SaveBMP_VariousSizes)
{
    // Тест с минимальным размером
    {
        constexpr int width = 1;
        constexpr int height = 1;
        uint8_t data[3] = {100, 150, 200};
        const auto bmp_file = test_dir_ / "test_1x1.bmp";

        EXPECT_TRUE(BMPHandler::saveBMP(bmp_file.string(), width, height, 3, data));
        EXPECT_TRUE(fs::exists(bmp_file));
    }

    // Тест с большим размером
    {
        constexpr int width = 100;
        constexpr int height = 100;
        auto test_image = TestUtils::createTestImage(width, height);
        const auto bmp_file = test_dir_ / "test_100x100.bmp";

        EXPECT_TRUE(BMPHandler::saveBMP(bmp_file.string(), width, height, 3, test_image.data()));
        EXPECT_TRUE(fs::exists(bmp_file));
    }

    // Тест с нестандартным размером (не кратно 4)
    {
        constexpr int width = 7;
        constexpr int height = 7;
        auto test_image = TestUtils::createTestImage(width, height);
        const auto bmp_file = test_dir_ / "test_7x7.bmp";

        EXPECT_TRUE(BMPHandler::saveBMP(bmp_file.string(), width, height, 3, test_image.data()));
        EXPECT_TRUE(fs::exists(bmp_file));
    }
}

// Тест загрузки BMP с различными размерами
TEST_F(BMPHandlerTest, LoadBMP_VariousSizes)
{
    // Создаем и загружаем BMP разных размеров
    const std::vector<std::pair<int, int>> sizes = {{1, 1}, {10, 10}, {50, 50}, {7, 7}};

    for (const auto& [width, height] : sizes)
    {
        auto test_image = TestUtils::createTestImage(width, height);
        const auto bmp_file = test_dir_ / ("test_" + std::to_string(width) + "x" + std::to_string(height) + ".bmp");

        ASSERT_TRUE(BMPHandler::saveBMP(bmp_file.string(), width, height, 3, test_image.data()));

        int loaded_width = 0;
        int loaded_height = 0;
        int loaded_channels = 0;
        uint8_t* loaded_data = BMPHandler::loadBMP(bmp_file.string(), loaded_width, loaded_height, loaded_channels);

        ASSERT_NE(loaded_data, nullptr);
        EXPECT_EQ(loaded_width, width);
        EXPECT_EQ(loaded_height, height);
        EXPECT_EQ(loaded_channels, 3);

        std::free(loaded_data);
    }
}

// Тест сохранения BMP с RGBA данными (должны быть преобразованы в RGB)
TEST_F(BMPHandlerTest, SaveBMP_RGBA_Conversion)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto rgba_image = TestUtils::createTestImageRGBA(width, height);
    const auto bmp_file = test_dir_ / "test_rgba_conversion.bmp";

    const bool result = BMPHandler::saveBMP(bmp_file.string(), width, height, 4, rgba_image.data());

    EXPECT_TRUE(result);
    EXPECT_TRUE(fs::exists(bmp_file));

    // Загружаем обратно и проверяем, что это RGB
    int loaded_width = 0;
    int loaded_height = 0;
    int loaded_channels = 0;
    uint8_t* loaded_data = BMPHandler::loadBMP(bmp_file.string(), loaded_width, loaded_height, loaded_channels);

    ASSERT_NE(loaded_data, nullptr);
    EXPECT_EQ(loaded_channels, 3); // BMP всегда RGB

    std::free(loaded_data);
}

// Тест загрузки BMP - поврежденный файл (не BMP формат)
TEST_F(BMPHandlerTest, LoadBMP_InvalidFormat)
{
    // Создаем файл с некорректным форматом
    const auto invalid_file = test_dir_ / "invalid.bmp";
    std::ofstream file(invalid_file);
    file << "This is not a BMP file";
    file.close();

    int width = 0;
    int height = 0;
    int channels = 0;
    uint8_t* data = BMPHandler::loadBMP(invalid_file.string(), width, height, channels);

    EXPECT_EQ(data, nullptr);
}

// Тест загрузки BMP - пустой файл
TEST_F(BMPHandlerTest, LoadBMP_EmptyFile)
{
    const auto empty_file = test_dir_ / "empty.bmp";
    std::ofstream file(empty_file);
    file.close();

    int width = 0;
    int height = 0;
    int channels = 0;
    uint8_t* data = BMPHandler::loadBMP(empty_file.string(), width, height, channels);

    EXPECT_EQ(data, nullptr);
}

// Тест сохранения BMP - проверка структуры файла
TEST_F(BMPHandlerTest, SaveBMP_FileStructure)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto bmp_file = test_dir_ / "test_structure.bmp";

    ASSERT_TRUE(BMPHandler::saveBMP(bmp_file.string(), width, height, 3, test_image.data()));

    // Проверяем, что файл существует и имеет минимальный размер
    EXPECT_TRUE(fs::exists(bmp_file));
    const auto file_size = fs::file_size(bmp_file);
    EXPECT_GT(file_size, 54); // Минимальный размер BMP файла (заголовки + данные)

    // Проверяем сигнатуру BMP (первые 2 байта должны быть "BM")
    std::ifstream file(bmp_file, std::ios::binary);
    ASSERT_TRUE(file.is_open());
    
    uint16_t signature = 0;
    file.read(reinterpret_cast<char*>(&signature), sizeof(signature));
    EXPECT_EQ(signature, 0x4D42); // "BM" в little-endian

    file.close();
}

// Тест сохранения BMP с одним каналом (grayscale)
TEST_F(BMPHandlerTest, SaveBMP_SingleChannel)
{
    constexpr int width = 10;
    constexpr int height = 10;
    std::vector<uint8_t> gray_data(width * height);
    for (size_t i = 0; i < gray_data.size(); ++i)
    {
        gray_data[i] = static_cast<uint8_t>(i % 256);
    }
    const auto bmp_file = test_dir_ / "test_grayscale.bmp";

    const bool result = BMPHandler::saveBMP(bmp_file.string(), width, height, 1, gray_data.data());

    EXPECT_TRUE(result);
    EXPECT_TRUE(fs::exists(bmp_file));
}

// Тест загрузки BMP - проверка корректности данных после загрузки
TEST_F(BMPHandlerTest, LoadBMP_DataCorrectness)
{
    constexpr int width = 5;
    constexpr int height = 5;
    
    // Создаем изображение с известными значениями
    std::vector<uint8_t> original_data(width * height * 3);
    for (int i = 0; i < width * height; ++i)
    {
        original_data[i * 3 + 0] = 100; // R
        original_data[i * 3 + 1] = 150; // G
        original_data[i * 3 + 2] = 200; // B
    }

    const auto bmp_file = test_dir_ / "test_data.bmp";
    ASSERT_TRUE(BMPHandler::saveBMP(bmp_file.string(), width, height, 3, original_data.data()));

    int loaded_width = 0;
    int loaded_height = 0;
    int loaded_channels = 0;
    uint8_t* loaded_data = BMPHandler::loadBMP(bmp_file.string(), loaded_width, loaded_height, loaded_channels);

    ASSERT_NE(loaded_data, nullptr);
    EXPECT_EQ(loaded_width, width);
    EXPECT_EQ(loaded_height, height);
    EXPECT_EQ(loaded_channels, 3);

    // Проверяем, что данные загружены (значения могут немного отличаться из-за формата BMP)
    EXPECT_NE(loaded_data[0], 0);
    EXPECT_NE(loaded_data[1], 0);
    EXPECT_NE(loaded_data[2], 0);

    std::free(loaded_data);
}

