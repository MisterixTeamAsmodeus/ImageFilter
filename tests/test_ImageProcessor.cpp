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
    EXPECT_FALSE(result.isSuccess());
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
    EXPECT_TRUE(processor.loadFromFile(test_file).isSuccess());
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
    EXPECT_EQ(processor.getChannels(), 3);
    EXPECT_FALSE(processor.hasAlpha());
    EXPECT_NE(processor.getData(), nullptr);

    // Сохраняем изображение
    const auto output_file = (test_dir_ / "test_output.png").string();
    EXPECT_TRUE(processor.saveToFile(output_file).isSuccess());
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
    ASSERT_TRUE(processor.loadFromFile(test_file).isSuccess());

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
    ASSERT_TRUE(processor.loadFromFile(test_file).isSuccess());

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
    EXPECT_TRUE(processor.resize(100, 200).isSuccess());
    EXPECT_EQ(processor.getWidth(), 100);
    EXPECT_EQ(processor.getHeight(), 200);
    EXPECT_FALSE(processor.isValid()); // Данных нет
}

// Тест метода resize с некорректными параметрами
TEST_F(ImageProcessorTest, ResizeInvalidParameters)
{
    ImageProcessor processor;
    
    EXPECT_FALSE(processor.resize(-1, 100).isSuccess());
    EXPECT_FALSE(processor.resize(100, -1).isSuccess());
    EXPECT_FALSE(processor.resize(0, 100).isSuccess());
    EXPECT_FALSE(processor.resize(100, 0).isSuccess());
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

    EXPECT_TRUE(processor.resize(new_width, new_height, allocated_data).isSuccess());
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
    EXPECT_FALSE(processor.saveToFile(output_file).isSuccess());
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
    ASSERT_TRUE(processor.loadFromFile(test_file1).isSuccess());
    EXPECT_EQ(processor.getWidth(), width1);
    EXPECT_EQ(processor.getHeight(), height1);

    // Загружаем второе изображение поверх первого
    ASSERT_TRUE(processor.loadFromFile(test_file2).isSuccess());
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
    EXPECT_TRUE(processor.loadFromFile(test_file, true).isSuccess());
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
    EXPECT_TRUE(processor.loadFromFile(test_file, true).isSuccess());
    EXPECT_EQ(processor.getChannels(), 4);
    EXPECT_TRUE(processor.hasAlpha());
    
    // Преобразуем в RGB
    EXPECT_TRUE(processor.convertToRGB().isSuccess());
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
    EXPECT_TRUE(processor.loadFromFile(test_file).isSuccess());
    EXPECT_EQ(processor.getChannels(), 3);
    
    // Попытка преобразовать RGB в RGB должна вернуть false
    EXPECT_FALSE(processor.convertToRGB().isSuccess());
    EXPECT_EQ(processor.getChannels(), 3);
}

// Тест преобразования невалидного изображения
TEST_F(ImageProcessorTest, ConvertInvalidImage)
{
    ImageProcessor processor;
    EXPECT_FALSE(processor.convertToRGB().isSuccess());
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
    EXPECT_FALSE(processor.loadFromFile(invalid_file).isSuccess());
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
    ASSERT_TRUE(processor.loadFromFile(test_file).isSuccess());
    
    // Пытаемся сохранить в несуществующую директорию
    const auto output_file = (test_dir_ / "nonexistent" / "output.png").string();
    // Это может либо создать директорию, либо вернуть ошибку - зависит от реализации
    // Просто проверяем, что не падает
    processor.saveToFile(output_file);
}

// Тест загрузки файла с нулевым размером
TEST_F(ImageProcessorTest, LoadEmptyFile)
{
    // Создаем пустой файл
    const auto empty_file = (test_dir_ / "empty.png").string();
    std::ofstream file(empty_file);
    file.close();
    
    ImageProcessor processor;
    EXPECT_FALSE(processor.loadFromFile(empty_file).isSuccess());
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
    
    EXPECT_TRUE(processor.resize(10, 10).isSuccess());
    EXPECT_EQ(processor.getWidth(), 10);
    EXPECT_EQ(processor.getHeight(), 10);
    
    EXPECT_TRUE(processor.resize(20, 20).isSuccess());
    EXPECT_EQ(processor.getWidth(), 20);
    EXPECT_EQ(processor.getHeight(), 20);
    
    EXPECT_TRUE(processor.resize(5, 5).isSuccess());
    EXPECT_EQ(processor.getWidth(), 5);
    EXPECT_EQ(processor.getHeight(), 5);
}

// Тест move-конструктора
TEST_F(ImageProcessorTest, MoveConstructor)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_move.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor1;
    ASSERT_TRUE(processor1.loadFromFile(test_file).isSuccess());
    EXPECT_TRUE(processor1.isValid());
    EXPECT_EQ(processor1.getWidth(), width);
    EXPECT_EQ(processor1.getHeight(), height);
    
    // Перемещаем данные
    ImageProcessor processor2(std::move(processor1));
    
    // Проверяем, что данные переместились
    EXPECT_TRUE(processor2.isValid());
    EXPECT_EQ(processor2.getWidth(), width);
    EXPECT_EQ(processor2.getHeight(), height);
    EXPECT_NE(processor2.getData(), nullptr);
    
    // Проверяем, что исходный объект стал невалидным
    EXPECT_FALSE(processor1.isValid());
    EXPECT_EQ(processor1.getWidth(), 0);
    EXPECT_EQ(processor1.getHeight(), 0);
    EXPECT_EQ(processor1.getData(), nullptr);
}

// Тест move-оператора присваивания
TEST_F(ImageProcessorTest, MoveAssignment)
{
    constexpr int width1 = 10;
    constexpr int height1 = 10;
    auto test_image1 = TestUtils::createTestImage(width1, height1);
    const auto filepath1 = test_dir_ / "test_move1.png";
    const auto test_file1 = TestUtils::saveTestImagePNG(test_image1, width1, height1, filepath1.string());
    ASSERT_FALSE(test_file1.empty());
    
    constexpr int width2 = 20;
    constexpr int height2 = 20;
    auto test_image2 = TestUtils::createTestImage(width2, height2);
    const auto filepath2 = test_dir_ / "test_move2.png";
    const auto test_file2 = TestUtils::saveTestImagePNG(test_image2, width2, height2, filepath2.string());
    ASSERT_FALSE(test_file2.empty());
    
    ImageProcessor processor1;
    ASSERT_TRUE(processor1.loadFromFile(test_file1).isSuccess());
    
    ImageProcessor processor2;
    ASSERT_TRUE(processor2.loadFromFile(test_file2).isSuccess());
    
    // Перемещаем данные из processor1 в processor2
    processor2 = std::move(processor1);
    
    // Проверяем, что processor2 получил данные из processor1
    EXPECT_TRUE(processor2.isValid());
    EXPECT_EQ(processor2.getWidth(), width1);
    EXPECT_EQ(processor2.getHeight(), height1);
    
    // Проверяем, что processor1 стал невалидным
    EXPECT_FALSE(processor1.isValid());
    EXPECT_EQ(processor1.getWidth(), 0);
    EXPECT_EQ(processor1.getHeight(), 0);
}

// Тест move-оператора присваивания с самоприсваиванием
TEST_F(ImageProcessorTest, MoveAssignmentSelf)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_move_self.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file).isSuccess());
    
    // Самоприсваивание не должно изменить объект
    ImageProcessor& processor_ref = processor;
    processor = std::move(processor_ref);
    
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест сохранения в JPEG формат
TEST_F(ImageProcessorTest, SaveToJPEG)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_input.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file).isSuccess());
    
    // Сохраняем в JPEG
    const auto output_file = (test_dir_ / "test_output.jpg").string();
    EXPECT_TRUE(processor.saveToFile(output_file).isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
}

// Тест сохранения в JPEG с различным качеством
TEST_F(ImageProcessorTest, SaveToJPEGWithQuality)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_input.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file).isSuccess());
    
    // Сохраняем с низким качеством
    processor.setJpegQuality(10);
    const auto output_low = (test_dir_ / "test_low.jpg").string();
    EXPECT_TRUE(processor.saveToFile(output_low).isSuccess());
    EXPECT_TRUE(fs::exists(output_low));
    
    // Сохраняем с высоким качеством
    processor.setJpegQuality(100);
    const auto output_high = (test_dir_ / "test_high.jpg").string();
    EXPECT_TRUE(processor.saveToFile(output_high).isSuccess());
    EXPECT_TRUE(fs::exists(output_high));
}

// Тест сохранения RGBA в PNG с альфа-каналом
TEST_F(ImageProcessorTest, SaveRGBAWithAlpha)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    const auto filepath = test_dir_ / "test_rgba_input.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file, true).isSuccess());
    EXPECT_EQ(processor.getChannels(), 4);
    EXPECT_TRUE(processor.hasAlpha());
    
    // Сохраняем с альфа-каналом
    const auto output_file = (test_dir_ / "test_rgba_output.png").string();
    EXPECT_TRUE(processor.saveToFile(output_file, true).isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
}

// Тест сохранения RGBA в RGB (без альфа-канала)
TEST_F(ImageProcessorTest, SaveRGBAWithoutAlpha)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    const auto filepath = test_dir_ / "test_rgba_input2.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file, true).isSuccess());
    EXPECT_EQ(processor.getChannels(), 4);
    
    // Сохраняем без альфа-канала
    const auto output_file = (test_dir_ / "test_rgb_output.png").string();
    EXPECT_TRUE(processor.saveToFile(output_file, false).isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
}

// Тест загрузки и сохранения очень маленького изображения (1x1)
TEST_F(ImageProcessorTest, VerySmallImage1x1)
{
    constexpr int width = 1;
    constexpr int height = 1;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_1x1.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    EXPECT_TRUE(processor.loadFromFile(test_file).isSuccess());
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
    EXPECT_NE(processor.getData(), nullptr);
    
    // Сохраняем обратно
    const auto output_file = (test_dir_ / "test_1x1_output.png").string();
    EXPECT_TRUE(processor.saveToFile(output_file).isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
}

// Тест загрузки и сохранения очень маленького изображения (1x2)
TEST_F(ImageProcessorTest, VerySmallImage1x2)
{
    constexpr int width = 1;
    constexpr int height = 2;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_1x2.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    EXPECT_TRUE(processor.loadFromFile(test_file).isSuccess());
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест загрузки и сохранения очень маленького изображения (2x1)
TEST_F(ImageProcessorTest, VerySmallImage2x1)
{
    constexpr int width = 2;
    constexpr int height = 1;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_2x1.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    EXPECT_TRUE(processor.loadFromFile(test_file).isSuccess());
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест convertToRGB с очень маленьким изображением
TEST_F(ImageProcessorTest, ConvertToRGBVerySmall)
{
    constexpr int width = 1;
    constexpr int height = 1;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    const auto filepath = test_dir_ / "test_rgba_1x1.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    EXPECT_TRUE(processor.loadFromFile(test_file, true).isSuccess());
    EXPECT_EQ(processor.getChannels(), 4);
    
    EXPECT_TRUE(processor.convertToRGB().isSuccess());
    EXPECT_EQ(processor.getChannels(), 3);
    EXPECT_FALSE(processor.hasAlpha());
}

// Тест convertToRGB с большим изображением
TEST_F(ImageProcessorTest, ConvertToRGBLarge)
{
    constexpr int width = 1000;
    constexpr int height = 1000;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    const auto filepath = test_dir_ / "test_rgba_large.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    EXPECT_TRUE(processor.loadFromFile(test_file, true).isSuccess());
    EXPECT_EQ(processor.getChannels(), 4);
    
    EXPECT_TRUE(processor.convertToRGB().isSuccess());
    EXPECT_EQ(processor.getChannels(), 3);
    EXPECT_FALSE(processor.hasAlpha());
}

// Тест resize с очень маленькими размерами
TEST_F(ImageProcessorTest, ResizeVerySmall)
{
    ImageProcessor processor;
    
    // Устанавливаем минимальные размеры
    EXPECT_TRUE(processor.resize(1, 1).isSuccess());
    EXPECT_EQ(processor.getWidth(), 1);
    EXPECT_EQ(processor.getHeight(), 1);
    EXPECT_FALSE(processor.isValid()); // Данных нет
    
    // Устанавливаем данные
    constexpr int width = 1;
    constexpr int height = 1;
    auto test_image = TestUtils::createTestImage(width, height);
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    EXPECT_TRUE(processor.resize(width, height, allocated_data).isSuccess());
    EXPECT_TRUE(processor.isValid());
}

// Тест resize с данными, но некорректным размером буфера
TEST_F(ImageProcessorTest, ResizeWithIncorrectBufferSize)
{
    ImageProcessor processor;
    
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    
    // Создаем буфер меньшего размера
    constexpr int smaller_size = width * height * 3 / 2;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(smaller_size));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), smaller_size);
    
    // Это может привести к проблемам, но resize не проверяет размер буфера
    // Просто проверяем, что не падает
    processor.resize(width, height, allocated_data);
    
    std::free(allocated_data);
}

// Тест обработки поврежденного файла (неполный PNG)
TEST_F(ImageProcessorTest, LoadCorruptedFile)
{
    // Создаем файл с неполными данными PNG
    const auto corrupted_file = (test_dir_ / "corrupted.png").string();
    std::ofstream file(corrupted_file, std::ios::binary);
    // Записываем только заголовок PNG без данных
    file.write("\x89PNG\r\n\x1a\n", 8);
    file.close();
    
    ImageProcessor processor;
    EXPECT_FALSE(processor.loadFromFile(corrupted_file).isSuccess());
    EXPECT_FALSE(processor.isValid());
}

// Тест обработки файла с неправильным расширением
TEST_F(ImageProcessorTest, LoadFileWithWrongExtension)
{
    // Создаем PNG файл с правильным расширением
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test.png";
    
    // Сохраняем через ImageProcessor напрямую
    ImageProcessor save_processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    ASSERT_TRUE(save_processor.resize(width, height, allocated_data).isSuccess());
    ASSERT_TRUE(save_processor.saveToFile(filepath.string()).isSuccess());
    ASSERT_TRUE(fs::exists(filepath));
    
    // Переименовываем файл в .txt
    const auto txt_file = test_dir_ / "test.txt";
    if (fs::exists(txt_file))
    {
        fs::remove(txt_file);
    }
    fs::rename(filepath, txt_file);
    ASSERT_TRUE(fs::exists(txt_file));
    
    ImageProcessor processor;
    // STB Image определяет формат по содержимому, а не по расширению
    // Поэтому это должно работать
    const auto result = processor.loadFromFile(txt_file.string());
    // Может работать или не работать в зависимости от реализации
    // Просто проверяем, что не падает
    (void)result; // Подавляем предупреждение о неиспользуемой переменной
}

// Тест сохранения в недоступную директорию (только чтение)
TEST_F(ImageProcessorTest, SaveToReadOnlyDirectory)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_input.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file).isSuccess());
    
    // Пытаемся сохранить в системную директорию (может не сработать из-за прав)
    const auto system_path = "/root/test_output.png";
    // Это может не сработать, но не должно падать
    processor.saveToFile(system_path);
}

// Тест множественных загрузок и сохранений
TEST_F(ImageProcessorTest, MultipleLoadSave)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_input.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    
    // Загружаем и сохраняем несколько раз
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_TRUE(processor.loadFromFile(test_file).isSuccess());
        const auto output_file = (test_dir_ / ("test_output_" + std::to_string(i) + ".png")).string();
        EXPECT_TRUE(processor.saveToFile(output_file).isSuccess());
        EXPECT_TRUE(fs::exists(output_file));
    }
}

// Тест convertToRGB после множественных операций
TEST_F(ImageProcessorTest, ConvertToRGBAfterOperations)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    
    // Сохраняем RGBA изображение напрямую через ImageProcessor
    ImageProcessor processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    EXPECT_TRUE(processor.resize(width, height, 4, allocated_data).isSuccess());
    EXPECT_EQ(processor.getChannels(), 4);
    EXPECT_TRUE(processor.hasAlpha());
    
    // Выполняем несколько операций с данными (симуляция обработки)
    // Просто проверяем, что данные остаются валидными
    EXPECT_NE(processor.getData(), nullptr);
    EXPECT_TRUE(processor.isValid());
    
    // Теперь конвертируем в RGB
    EXPECT_TRUE(processor.convertToRGB().isSuccess());
    EXPECT_EQ(processor.getChannels(), 3);
    EXPECT_FALSE(processor.hasAlpha());
    EXPECT_TRUE(processor.isValid());
}

// Тест обработки изображения с максимальными размерами (граничный случай)
TEST_F(ImageProcessorTest, MaxSizeImage)
{
    ImageProcessor processor;
    
    // Пытаемся создать изображение с очень большими размерами
    // Это может не сработать из-за ограничений памяти, но не должно падать
    constexpr int max_width = 100000;
    constexpr int max_height = 100000;
    
    // Просто проверяем, что resize не падает
    const auto result = processor.resize(max_width, max_height);
    // Может быть успешным или неуспешным в зависимости от доступной памяти
    EXPECT_EQ(processor.getWidth(), max_width);
    EXPECT_EQ(processor.getHeight(), max_height);
}

// ========== ФАЗА 3.1: Тесты для условных ветвлений ==========

// Тест самоприсваивания в move операторе (if (this != &other))
TEST_F(ImageProcessorTest, MoveAssignmentSelfAssignment)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_move_self.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file).isSuccess());
    
    // Самоприсваивание через move не должно изменить объект
    ImageProcessor& ref = processor;
    processor = std::move(ref);
    
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), width);
    EXPECT_EQ(processor.getHeight(), height);
}

// Тест освобождения данных при загрузке нового изображения (if (data_ != nullptr))
TEST_F(ImageProcessorTest, LoadOverwritesExistingData)
{
    constexpr int width1 = 5;
    constexpr int height1 = 5;
    auto test_image1 = TestUtils::createTestImage(width1, height1);
    const auto filepath1 = test_dir_ / "test1.png";
    const auto test_file1 = TestUtils::saveTestImagePNG(test_image1, width1, height1, filepath1.string());
    ASSERT_FALSE(test_file1.empty());
    
    constexpr int width2 = 20;
    constexpr int height2 = 20;
    auto test_image2 = TestUtils::createTestImage(width2, height2);
    const auto filepath2 = test_dir_ / "test2.png";
    const auto test_file2 = TestUtils::saveTestImagePNG(test_image2, width2, height2, filepath2.string());
    ASSERT_FALSE(test_file2.empty());
    
    ImageProcessor processor;
    
    // Загружаем первое изображение
    ASSERT_TRUE(processor.loadFromFile(test_file1).isSuccess());
    const auto* data1 = processor.getData();
    EXPECT_NE(data1, nullptr);
    EXPECT_EQ(processor.getWidth(), width1);
    EXPECT_EQ(processor.getHeight(), height1);
    
    // Загружаем второе изображение - должно освободить первое
    ASSERT_TRUE(processor.loadFromFile(test_file2).isSuccess());
    const auto* data2 = processor.getData();
    EXPECT_NE(data2, nullptr);
    EXPECT_EQ(processor.getWidth(), width2);
    EXPECT_EQ(processor.getHeight(), height2);
    // Указатели должны быть разными (старые данные освобождены)
    EXPECT_NE(data1, data2);
}

// Тест обработки ошибки загрузки (if (!result.isSuccess()))
TEST_F(ImageProcessorTest, LoadErrorHandling)
{
    ImageProcessor processor;
    
    // Пытаемся загрузить несуществующий файл
    const auto result = processor.loadFromFile("nonexistent_file_12345.png");
    EXPECT_FALSE(result.isSuccess());
    EXPECT_FALSE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), 0);
    EXPECT_EQ(processor.getHeight(), 0);
    EXPECT_EQ(processor.getChannels(), 0);
    EXPECT_EQ(processor.getData(), nullptr);
}

// Тест сохранения невалидного изображения (if (!isValid()))
TEST_F(ImageProcessorTest, SaveInvalidImage)
{
    ImageProcessor processor;
    const auto output_file = (test_dir_ / "invalid_output.png").string();
    
    // Пытаемся сохранить невалидное изображение
    const auto result = processor.saveToFile(output_file);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
    EXPECT_FALSE(fs::exists(output_file));
}

// Тест установки качества JPEG на граничных значениях (if (quality < 0 || quality > 100))
TEST_F(ImageProcessorTest, SetJpegQualityBoundaries)
{
    ImageProcessor processor;
    
    // Граничные значения: 0 и 100 должны быть валидными
    EXPECT_TRUE(processor.setJpegQuality(0));
    EXPECT_EQ(processor.getJpegQuality(), 0);
    
    EXPECT_TRUE(processor.setJpegQuality(100));
    EXPECT_EQ(processor.getJpegQuality(), 100);
    
    // Значения вне диапазона должны быть невалидными
    EXPECT_FALSE(processor.setJpegQuality(-1));
    EXPECT_EQ(processor.getJpegQuality(), 100); // Значение не должно измениться
    
    EXPECT_FALSE(processor.setJpegQuality(101));
    EXPECT_EQ(processor.getJpegQuality(), 100); // Значение не должно измениться
    
    EXPECT_FALSE(processor.setJpegQuality(-100));
    EXPECT_EQ(processor.getJpegQuality(), 100);
    
    EXPECT_FALSE(processor.setJpegQuality(1000));
    EXPECT_EQ(processor.getJpegQuality(), 100);
}

// Тест convertToRGB с невалидным изображением (if (!isValid() || channels_ != 4))
TEST_F(ImageProcessorTest, ConvertToRGBInvalidImage)
{
    ImageProcessor processor;
    
    // Пытаемся конвертировать невалидное изображение
    const auto result1 = processor.convertToRGB();
    EXPECT_FALSE(result1.isSuccess());
    EXPECT_EQ(result1.error, FilterError::InvalidImage);
}

// Тест convertToRGB с RGB изображением (channels_ != 4)
TEST_F(ImageProcessorTest, ConvertToRGBFromRGB)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = test_dir_ / "test_rgb.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file).isSuccess());
    EXPECT_EQ(processor.getChannels(), 3);
    
    // Пытаемся конвертировать RGB в RGB - должно вернуть ошибку
    const auto result = processor.convertToRGB();
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
    EXPECT_EQ(processor.getChannels(), 3); // Каналы не должны измениться
}

// Тест convertToRGB с нулевыми размерами (if (width_ <= 0 || height_ <= 0))
TEST_F(ImageProcessorTest, ConvertToRGBZeroSize)
{
    ImageProcessor processor;
    
    // Создаем RGBA изображение с нулевыми размерами (не должно быть возможно, но тестируем)
    // В реальности это не должно произойти, но проверяем ветвь
    processor.resize(0, 0);
    EXPECT_FALSE(processor.isValid());
    
    // Попытка конвертации должна вернуть ошибку
    const auto result = processor.convertToRGB();
    EXPECT_FALSE(result.isSuccess());
}

// Тест convertToRGB с отрицательными размерами
TEST_F(ImageProcessorTest, ConvertToRGBNegativeSize)
{
    ImageProcessor processor;
    
    // Создаем RGBA изображение с отрицательными размерами
    processor.resize(-1, -1);
    EXPECT_FALSE(processor.isValid());
    
    // Попытка конвертации должна вернуть ошибку
    const auto result = processor.convertToRGB();
    EXPECT_FALSE(result.isSuccess());
}

// Тест resize с нулевыми размерами (if (new_width <= 0 || new_height <= 0))
TEST_F(ImageProcessorTest, ResizeZeroDimensions)
{
    ImageProcessor processor;
    
    // Тест с нулевой шириной
    EXPECT_FALSE(processor.resize(0, 10).isSuccess());
    EXPECT_EQ(processor.resize(0, 10).error, FilterError::InvalidSize);
    
    // Тест с нулевой высотой
    EXPECT_FALSE(processor.resize(10, 0).isSuccess());
    EXPECT_EQ(processor.resize(10, 0).error, FilterError::InvalidSize);
    
    // Тест с обеими нулевыми размерами
    EXPECT_FALSE(processor.resize(0, 0).isSuccess());
    EXPECT_EQ(processor.resize(0, 0).error, FilterError::InvalidSize);
}

// Тест resize с отрицательными размерами
TEST_F(ImageProcessorTest, ResizeNegativeDimensions)
{
    ImageProcessor processor;
    
    // Тест с отрицательной шириной
    EXPECT_FALSE(processor.resize(-1, 10).isSuccess());
    EXPECT_EQ(processor.resize(-1, 10).error, FilterError::InvalidSize);
    
    // Тест с отрицательной высотой
    EXPECT_FALSE(processor.resize(10, -1).isSuccess());
    EXPECT_EQ(processor.resize(10, -1).error, FilterError::InvalidSize);
    
    // Тест с обеими отрицательными размерами
    EXPECT_FALSE(processor.resize(-1, -1).isSuccess());
    EXPECT_EQ(processor.resize(-1, -1).error, FilterError::InvalidSize);
}

// Тест resize с channels_ == 0 (установка каналов по умолчанию)
TEST_F(ImageProcessorTest, ResizeWithZeroChannels)
{
    ImageProcessor processor;
    
    // Создаем изображение без данных, channels_ будет 0
    EXPECT_TRUE(processor.resize(10, 10).isSuccess());
    EXPECT_EQ(processor.getWidth(), 10);
    EXPECT_EQ(processor.getHeight(), 10);
    // channels_ должен быть установлен в 3 по умолчанию
    EXPECT_EQ(processor.getChannels(), 3);
}

// Тест resize с new_data == nullptr (if (new_data == nullptr))
TEST_F(ImageProcessorTest, ResizeWithNullData)
{
    ImageProcessor processor;
    
    // Resize с nullptr должен просто установить размеры без данных
    EXPECT_TRUE(processor.resize(100, 200, nullptr).isSuccess());
    EXPECT_EQ(processor.getWidth(), 100);
    EXPECT_EQ(processor.getHeight(), 200);
    EXPECT_FALSE(processor.isValid()); // Данных нет
    EXPECT_EQ(processor.getData(), nullptr);
}

// Тест resize с некорректными каналами (if (new_channels != 3 && new_channels != 4))
TEST_F(ImageProcessorTest, ResizeWithInvalidChannels)
{
    ImageProcessor processor;
    
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    // Тест с каналами = 0
    EXPECT_FALSE(processor.resize(width, height, 0, allocated_data).isSuccess());
    EXPECT_EQ(processor.resize(width, height, 0, allocated_data).error, FilterError::InvalidSize);
    
    // Тест с каналами = 1
    EXPECT_FALSE(processor.resize(width, height, 1, allocated_data).isSuccess());
    
    // Тест с каналами = 2
    EXPECT_FALSE(processor.resize(width, height, 2, allocated_data).isSuccess());
    
    // Тест с каналами = 5
    EXPECT_FALSE(processor.resize(width, height, 5, allocated_data).isSuccess());
    
    // Тест с каналами = -1
    EXPECT_FALSE(processor.resize(width, height, -1, allocated_data).isSuccess());
    
    std::free(allocated_data);
}

// Тест resize с new_data == nullptr для версии с каналами
TEST_F(ImageProcessorTest, ResizeWithChannelsNullData)
{
    ImageProcessor processor;
    
    // Resize с nullptr и указанием каналов должен установить размеры и каналы
    EXPECT_TRUE(processor.resize(50, 75, 4, nullptr).isSuccess());
    EXPECT_EQ(processor.getWidth(), 50);
    EXPECT_EQ(processor.getHeight(), 75);
    EXPECT_EQ(processor.getChannels(), 4);
    EXPECT_FALSE(processor.isValid()); // Данных нет
    EXPECT_EQ(processor.getData(), nullptr);
}

// ========== ФАЗА 3.2: Тесты для граничных значений ==========

// Тест минимальных значений размеров (1x1)
TEST_F(ImageProcessorTest, BoundaryMinSize1x1)
{
    ImageProcessor processor;
    
    // Минимальный размер: 1x1
    EXPECT_TRUE(processor.resize(1, 1).isSuccess());
    EXPECT_EQ(processor.getWidth(), 1);
    EXPECT_EQ(processor.getHeight(), 1);
    
    // Создаем изображение 1x1 с данными
    constexpr int width = 1;
    constexpr int height = 1;
    auto test_image = TestUtils::createTestImage(width, height);
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    EXPECT_TRUE(processor.resize(width, height, allocated_data).isSuccess());
    EXPECT_TRUE(processor.isValid());
    EXPECT_EQ(processor.getWidth(), 1);
    EXPECT_EQ(processor.getHeight(), 1);
}

// Тест минимальных значений размеров (1x2 и 2x1)
TEST_F(ImageProcessorTest, BoundaryMinSize1x2And2x1)
{
    ImageProcessor processor;
    
    // Минимальный размер: 1x2
    EXPECT_TRUE(processor.resize(1, 2).isSuccess());
    EXPECT_EQ(processor.getWidth(), 1);
    EXPECT_EQ(processor.getHeight(), 2);
    
    // Минимальный размер: 2x1
    EXPECT_TRUE(processor.resize(2, 1).isSuccess());
    EXPECT_EQ(processor.getWidth(), 2);
    EXPECT_EQ(processor.getHeight(), 1);
}

// Тест граничных значений качества JPEG (0, 1, 99, 100)
TEST_F(ImageProcessorTest, BoundaryJpegQuality)
{
    ImageProcessor processor;
    
    // Минимальное значение: 0
    EXPECT_TRUE(processor.setJpegQuality(0));
    EXPECT_EQ(processor.getJpegQuality(), 0);
    
    // Граничное значение: 1
    EXPECT_TRUE(processor.setJpegQuality(1));
    EXPECT_EQ(processor.getJpegQuality(), 1);
    
    // Граничное значение: 99
    EXPECT_TRUE(processor.setJpegQuality(99));
    EXPECT_EQ(processor.getJpegQuality(), 99);
    
    // Максимальное значение: 100
    EXPECT_TRUE(processor.setJpegQuality(100));
    EXPECT_EQ(processor.getJpegQuality(), 100);
    
    // Значения за границами: -1 и 101
    EXPECT_FALSE(processor.setJpegQuality(-1));
    EXPECT_FALSE(processor.setJpegQuality(101));
}

// Тест граничных значений каналов (3 и 4)
TEST_F(ImageProcessorTest, BoundaryChannels)
{
    ImageProcessor processor;
    
    constexpr int width = 10;
    constexpr int height = 10;
    
    // Тест с 3 каналами (RGB)
    auto test_image_rgb = TestUtils::createTestImage(width, height);
    auto* rgb_data = static_cast<uint8_t*>(std::malloc(test_image_rgb.size()));
    ASSERT_NE(rgb_data, nullptr);
    std::memcpy(rgb_data, test_image_rgb.data(), test_image_rgb.size());
    
    EXPECT_TRUE(processor.resize(width, height, 3, rgb_data).isSuccess());
    EXPECT_EQ(processor.getChannels(), 3);
    EXPECT_FALSE(processor.hasAlpha());
    
    // Тест с 4 каналами (RGBA)
    auto test_image_rgba = TestUtils::createTestImageRGBA(width, height);
    auto* rgba_data = static_cast<uint8_t*>(std::malloc(test_image_rgba.size()));
    ASSERT_NE(rgba_data, nullptr);
    std::memcpy(rgba_data, test_image_rgba.data(), test_image_rgba.size());
    
    EXPECT_TRUE(processor.resize(width, height, 4, rgba_data).isSuccess());
    EXPECT_EQ(processor.getChannels(), 4);
    EXPECT_TRUE(processor.hasAlpha());
}

// Тест граничных значений размеров (очень большие, но не вызывающие переполнение)
TEST_F(ImageProcessorTest, BoundaryLargeSizes)
{
    ImageProcessor processor;
    
    // Большие, но разумные размеры
    constexpr int large_width = 10000;
    constexpr int large_height = 10000;
    
    // Resize без данных должен работать
    EXPECT_TRUE(processor.resize(large_width, large_height).isSuccess());
    EXPECT_EQ(processor.getWidth(), large_width);
    EXPECT_EQ(processor.getHeight(), large_height);
}

// Тест граничных значений для convertToRGB (RGBA с минимальными размерами)
TEST_F(ImageProcessorTest, BoundaryConvertToRGBMinSize)
{
    constexpr int width = 1;
    constexpr int height = 1;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    const auto filepath = test_dir_ / "test_rgba_1x1.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file, true).isSuccess());
    EXPECT_EQ(processor.getChannels(), 4);
    
    // Конвертация должна работать даже для минимального размера
    EXPECT_TRUE(processor.convertToRGB().isSuccess());
    EXPECT_EQ(processor.getChannels(), 3);
    EXPECT_FALSE(processor.hasAlpha());
}

// Тест граничных значений для convertToRGB (RGBA с размерами 1x2)
TEST_F(ImageProcessorTest, BoundaryConvertToRGB1x2)
{
    constexpr int width = 1;
    constexpr int height = 2;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    const auto filepath = test_dir_ / "test_rgba_1x2.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file, true).isSuccess());
    EXPECT_EQ(processor.getChannels(), 4);
    
    EXPECT_TRUE(processor.convertToRGB().isSuccess());
    EXPECT_EQ(processor.getChannels(), 3);
}

// Тест граничных значений для convertToRGB (RGBA с размерами 2x1)
TEST_F(ImageProcessorTest, BoundaryConvertToRGB2x1)
{
    constexpr int width = 2;
    constexpr int height = 1;
    auto test_image = TestUtils::createTestImageRGBA(width, height);
    const auto filepath = test_dir_ / "test_rgba_2x1.png";
    const auto test_file = TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    ASSERT_FALSE(test_file.empty());
    
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file, true).isSuccess());
    EXPECT_EQ(processor.getChannels(), 4);
    
    EXPECT_TRUE(processor.convertToRGB().isSuccess());
    EXPECT_EQ(processor.getChannels(), 3);
}

// Тест граничных значений: нулевой указатель на данные
TEST_F(ImageProcessorTest, BoundaryNullDataPointer)
{
    ImageProcessor processor;
    
    // Resize с nullptr должен работать
    EXPECT_TRUE(processor.resize(10, 10, nullptr).isSuccess());
    EXPECT_FALSE(processor.isValid());
    EXPECT_EQ(processor.getData(), nullptr);
    
    // Resize с каналами и nullptr
    EXPECT_TRUE(processor.resize(20, 20, 3, nullptr).isSuccess());
    EXPECT_FALSE(processor.isValid());
    EXPECT_EQ(processor.getData(), nullptr);
}

// Тест граничных значений: пустая строка для имени файла
TEST_F(ImageProcessorTest, BoundaryEmptyFilename)
{
    ImageProcessor processor;
    
    // Загрузка с пустым именем файла
    const auto load_result = processor.loadFromFile("");
    EXPECT_FALSE(load_result.isSuccess());
    EXPECT_FALSE(processor.isValid());
    
    // Сохранение с пустым именем файла
    const auto save_result = processor.saveToFile("");
    EXPECT_FALSE(save_result.isSuccess());
}

// Тест граничных значений: значения на границе диапазона размеров (INT_MAX)
TEST_F(ImageProcessorTest, BoundaryMaxIntSize)
{
    ImageProcessor processor;
    
    // Пытаемся установить максимальные значения int
    // Это может вызвать переполнение при вычислении размера
    constexpr int max_int = std::numeric_limits<int>::max();
    
    // Resize без данных может установить размеры, но не выделить память
    const auto result = processor.resize(max_int, max_int);
    // Может быть успешным или неуспешным в зависимости от проверки переполнения
    // Проверяем, что не падает
    (void)result;
}

// Тест граничных значений: значения на границе диапазона размеров (INT_MAX / 2)
TEST_F(ImageProcessorTest, BoundaryLargeButSafeSize)
{
    ImageProcessor processor;
    
    // Используем большие, но безопасные значения
    // INT_MAX / 2 все еще может вызвать переполнение при умножении на каналы
    constexpr int large_safe = std::numeric_limits<int>::max() / 4;
    
    // Resize без данных должен работать
    EXPECT_TRUE(processor.resize(large_safe, large_safe).isSuccess());
    EXPECT_EQ(processor.getWidth(), large_safe);
    EXPECT_EQ(processor.getHeight(), large_safe);
}
