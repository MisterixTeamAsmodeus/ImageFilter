#include <gtest/gtest.h>
#include <ImageProcessor.h>
#include "test_utils.h"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief Тесты для класса ImageProcessor
 */
class ImageProcessorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Создаем временную директорию для тестовых файлов
        test_dir_ = fs::temp_directory_path() / "ImageFilterTests";
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

// Тест конструктора по умолчанию
TEST_F(ImageProcessorTest, DefaultConstructor)
{
    ImageProcessor processor;
    EXPECT_FALSE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), 0);
    EXPECT_EQ(processor.getHeight(), 0);
    EXPECT_EQ(processor.getChannels(), 0);
    EXPECT_EQ(processor.getData(), nullptr);
}

// Тест загрузки несуществующего файла
TEST_F(ImageProcessorTest, LoadNonExistentFile)
{
    ImageProcessor processor;
    const auto result = processor.loadFromFile("nonexistent_file.jpg");
    EXPECT_FALSE(result);
    EXPECT_FALSE(processor.isValid());
}

// Тест загрузки и сохранения PNG
TEST_F(ImageProcessorTest, LoadAndSavePNG)
{
    // Создаем тестовое изображение
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_input.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());

    ImageProcessor processor;
    
    // Загружаем изображение
    EXPECT_TRUE(processor.loadFromFile(test_file));
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
    EXPECT_EQ(processor.getChannels(), 3);
    EXPECT_FALSE(processor.hasAlpha());
    EXPECT_NE(processor.getData(), nullptr);

    // Сохраняем изображение
    const auto output_file = (test_dir_ / "test_output.png").string();
    EXPECT_TRUE(processor.saveToFile(output_file));
    EXPECT_TRUE(fs::exists(output_file));

    // Проверяем, что файл создан и не пустой
    EXPECT_GT(fs::file_size(output_file), 0);
}

// Тест получения размеров
TEST_F(ImageProcessorTest, GetDimensions)
{
    constexpr int width = 5;
    constexpr int height = 7;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_dimensions.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());

    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file));

    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
    EXPECT_EQ(processor.getChannels(), 3);
    EXPECT_FALSE(processor.hasAlpha());
}

// Тест получения данных
TEST_F(ImageProcessorTest, GetData)
{
    constexpr int width = 3;
    constexpr int height = 3;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_data.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());

    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file));

    const auto* const_data = processor.getData();
    auto* mutable_data = processor.getData();

    EXPECT_NE(const_data, nullptr);
    EXPECT_NE(mutable_data, nullptr);
    EXPECT_EQ(const_data, mutable_data);

    // Проверяем, что данные можно изменять
    const auto original_value = mutable_data[0];
    mutable_data[0] = 255;
    EXPECT_EQ(mutable_data[0], 255);
    mutable_data[0] = original_value; // Восстанавливаем
}

// Тест метода resize без данных
TEST_F(ImageProcessorTest, ResizeWithoutData)
{
    ImageProcessor processor;
    
    // Resize без данных должен установить размеры
    EXPECT_TRUE(processor.resize(100, 200));
    EXPECT_EQ(processor.getWidth(), 100);
    EXPECT_EQ(processor.getHeight(), 200);
    EXPECT_FALSE(processor.isValid()); // Данных нет
}

// Тест метода resize с некорректными параметрами
TEST_F(ImageProcessorTest, ResizeInvalidParameters)
{
    ImageProcessor processor;
    
    EXPECT_FALSE(processor.resize(-1, 100));
    EXPECT_FALSE(processor.resize(100, -1));
    EXPECT_FALSE(processor.resize(0, 100));
    EXPECT_FALSE(processor.resize(100, 0));
}

// Тест метода resize с данными
TEST_F(ImageProcessorTest, ResizeWithData)
{
    constexpr int width = 5;
    constexpr int height = 5;
    auto test_image = TestUtils::createTestImage(width, height);

    ImageProcessor processor;
    
    // Создаем новые данные для resize
    constexpr int new_width = 10;
    constexpr int new_height = 10;
    auto new_data = TestUtils::createTestImage(new_width, new_height);
    
    // Выделяем память через malloc (совместимо с stbi_image_free)
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(new_data.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, new_data.data(), new_data.size());

    EXPECT_TRUE(processor.resize(new_width, new_height, allocated_data));
    EXPECT_EQ(processor.getWidth(), new_width);
    EXPECT_EQ(processor.getHeight(), new_height);
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getChannels(), 3);
    EXPECT_FALSE(processor.hasAlpha());
}

// Тест сохранения без загруженного изображения
TEST_F(ImageProcessorTest, SaveWithoutImage)
{
    ImageProcessor processor;
    const auto output_file = (test_dir_ / "empty_output.png").string();
    EXPECT_FALSE(processor.saveToFile(output_file));
}

// Тест загрузки изображения поверх существующего
TEST_F(ImageProcessorTest, LoadOverExisting)
{
    constexpr int width1 = 5;
    constexpr int height1 = 5;
    auto test_image1 = TestUtils::createTestImage(width1, height1);
    const auto filepath1 = test_dir_ / "test1.png";
    const auto test_file1 = TestUtils::saveTestImagePNG(test_image1, width1, height1, filepath1.string());
    ASSERT_FALSE(test_file1.empty());

    constexpr int width2 = 10;
    constexpr int height2 = 10;
    auto test_image2 = TestUtils::createTestImage(width2, height2);
    const auto filepath2 = test_dir_ / "test2.png";
    const auto test_file2 = TestUtils::saveTestImagePNG(test_image2, width2, height2, filepath2.string());
    ASSERT_FALSE(test_file2.empty());

    ImageProcessor processor;
    
    // Загружаем первое изображение
    ASSERT_TRUE(processor.loadFromFile(test_file1));
    EXPECT_EQ(processor.getWidth(), width1);
    EXPECT_EQ(processor.getHeight(), height1);

    // Загружаем второе изображение поверх первого
    ASSERT_TRUE(processor.loadFromFile(test_file2));
    EXPECT_EQ(processor.getWidth(), width2);
    EXPECT_EQ(processor.getHeight(), height2);
    EXPECT_EQ(processor.getChannels(), 3);
    EXPECT_FALSE(processor.hasAlpha());
}

// Тест загрузки с альфа-каналом
TEST_F(ImageProcessorTest, LoadWithAlpha)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    const auto filepath = test_dir_ / "test_rgba.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());

    ImageProcessor processor;
    
    // Загружаем изображение с альфа-каналом
    EXPECT_TRUE(processor.loadFromFile(test_file, true));
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
    EXPECT_EQ(processor.getChannels(), 4);
    EXPECT_TRUE(processor.hasAlpha());
    EXPECT_NE(processor.getData(), nullptr);
}

// Тест преобразования RGBA в RGB
TEST_F(ImageProcessorTest, ConvertToRGB)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    
    // Используем временный файл для загрузки RGBA
    const auto filepath = test_dir_ / "test_rgba_convert.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    EXPECT_TRUE(processor.loadFromFile(test_file, true));
    EXPECT_EQ(processor.getChannels(), 4);
    EXPECT_TRUE(processor.hasAlpha());
    
    // Преобразуем в RGB
    EXPECT_TRUE(processor.convertToRGB());
    EXPECT_EQ(processor.getChannels(), 3);
    EXPECT_FALSE(processor.hasAlpha());
}

// Тест преобразования RGB в RGB (не должно ничего делать)
TEST_F(ImageProcessorTest, ConvertRGBToRGB)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_rgb.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    EXPECT_TRUE(processor.loadFromFile(test_file));
    EXPECT_EQ(processor.getChannels(), 3);
    
    // Попытка преобразовать RGB в RGB должна вернуть false
    EXPECT_FALSE(processor.convertToRGB());
    EXPECT_EQ(processor.getChannels(), 3);
}

// Тест преобразования невалидного изображения
TEST_F(ImageProcessorTest, ConvertInvalidImage)
{
    ImageProcessor processor;
    EXPECT_FALSE(processor.convertToRGB());
}

// Тест установки качества JPEG
TEST_F(ImageProcessorTest, SetJpegQuality)
{
    ImageProcessor processor;
    
    // Валидные значения
    EXPECT_TRUE(processor.setJpegQuality(50));
    EXPECT_EQ(processor.getJpegQuality(), 50);
    
    EXPECT_TRUE(processor.setJpegQuality(0));
    EXPECT_EQ(processor.getJpegQuality(), 0);
    
    EXPECT_TRUE(processor.setJpegQuality(100));
    EXPECT_EQ(processor.getJpegQuality(), 100);
    
    // Некорректные значения
    EXPECT_FALSE(processor.setJpegQuality(-1));
    EXPECT_FALSE(processor.setJpegQuality(101));
    
    // Значение должно остаться прежним при ошибке
    EXPECT_EQ(processor.getJpegQuality(), 100);
}

// Тест загрузки файла с некорректным форматом
TEST_F(ImageProcessorTest, LoadInvalidFormat)
{
    // Создаем текстовый файл вместо изображения
    const auto invalid_file = (test_dir_ / "invalid.txt").string();
    std::ofstream file(invalid_file);
    file << "This is not an image file";
    file.close();
    
    ImageProcessor processor;
    EXPECT_FALSE(processor.loadFromFile(invalid_file));
    EXPECT_FALSE(processor.isValid());
}

// Тест сохранения в несуществующую директорию
TEST_F(ImageProcessorTest, SaveToNonExistentDirectory)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_input.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file));
    
    // Пытаемся сохранить в несуществующую директорию
    const auto output_file = (test_dir_ / "nonexistent" / "output.png").string();
    // Это может либо создать директорию, либо вернуть ошибку - зависит от реализации
    // Просто проверяем, что не падает
    processor.saveToFile(output_file);
}

// Тест resize с некорректными каналами
TEST_F(ImageProcessorTest, ResizeWithInvalidChannels)
{
    ImageProcessor processor;
    
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    // Некорректное количество каналов
    EXPECT_FALSE(processor.resize(width, height, 0, allocated_data));
    EXPECT_FALSE(processor.resize(width, height, 2, allocated_data));
    EXPECT_FALSE(processor.resize(width, height, 5, allocated_data));
    
    std::free(allocated_data);
}

// Тест загрузки файла с нулевым размером
TEST_F(ImageProcessorTest, LoadEmptyFile)
{
    // Создаем пустой файл
    const auto empty_file = (test_dir_ / "empty.png").string();
    std::ofstream file(empty_file);
    file.close();
    
    ImageProcessor processor;
    EXPECT_FALSE(processor.loadFromFile(empty_file));
    EXPECT_FALSE(processor.isValid());
}

// Тест работы с очень большими размерами
TEST_F(ImageProcessorTest, VeryLargeImage)
{
    ImageProcessor processor;
    
    // Пытаемся создать очень большое изображение
    // Это может не сработать из-за ограничений памяти, но не должно падать
    constexpr int very_large_width = 10000;
    constexpr int very_large_height = 10000;
    
    // Просто проверяем, что resize не падает
    processor.resize(very_large_width, very_large_height);
    EXPECT_EQ(processor.getWidth(), very_large_width);
    EXPECT_EQ(processor.getHeight(), very_large_height);
}

// Тест множественных операций resize
TEST_F(ImageProcessorTest, MultipleResize)
{
    ImageProcessor processor;
    
    EXPECT_TRUE(processor.resize(10, 10));
    EXPECT_EQ(processor.getWidth(), 10);
    EXPECT_EQ(processor.getHeight(), 10);
    
    EXPECT_TRUE(processor.resize(20, 20));
    EXPECT_EQ(processor.getWidth(), 20);
    EXPECT_EQ(processor.getHeight(), 20);
    
    EXPECT_TRUE(processor.resize(5, 5));
    EXPECT_EQ(processor.getWidth(), 5);
    EXPECT_EQ(processor.getHeight(), 5);
}
