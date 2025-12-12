#include <gtest/gtest.h>
#include <utils/ImageValidator.h>
#include <ImageProcessor.h>
#include "test_utils.h"
#include <cstdlib>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief Тесты для ImageValidator
 */
class ImageValidatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Создаем временную директорию для тестовых файлов
        test_dir_ = fs::temp_directory_path() / "ImageValidatorTests";
        fs::create_directories(test_dir_);

        // Создаем тестовое изображение
        constexpr int width = 10;
        constexpr int height = 10;
        test_image_ = TestUtils::createTestImage(width, height);
        test_file_ = test_dir_ / "test_image.png";
        const auto saved_file = TestUtils::saveTestImagePNG(test_image_, width, height, test_file_.string());
        ASSERT_FALSE(saved_file.empty());
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
    fs::path test_file_;
    std::vector<uint8_t> test_image_;
};

// Тест базовой валидации - успешный случай
TEST_F(ImageValidatorTest, ValidateBasic_Success)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const auto result = ImageValidator::validateBasic(processor);
    EXPECT_TRUE(result.isSuccess());
}

// Тест базовой валидации - невалидное изображение
TEST_F(ImageValidatorTest, ValidateBasic_InvalidImage)
{
    ImageProcessor processor;
    // Изображение не загружено

    const auto result = ImageValidator::validateBasic(processor);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест валидации указателя на данные - успешный случай
TEST_F(ImageValidatorTest, ValidateDataPointer_Success)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const auto result = ImageValidator::validateDataPointer(
        processor.getData(),
        processor.getWidth(),
        processor.getHeight(),
        processor.getChannels()
    );
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации указателя на данные - nullptr
TEST_F(ImageValidatorTest, ValidateDataPointer_Nullptr)
{
    const auto result = ImageValidator::validateDataPointer(nullptr, 10, 10, 3);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImageData);
}

// Тест валидации размера данных - успешный случай
TEST_F(ImageValidatorTest, ValidateDataSize_Success)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const auto result = ImageValidator::validateDataSize(processor);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации размера данных - невалидное изображение
TEST_F(ImageValidatorTest, ValidateDataSize_InvalidImage)
{
    ImageProcessor processor;
    // Изображение не загружено

    const auto result = ImageValidator::validateDataSize(processor);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест валидации на пустоту - успешный случай
TEST_F(ImageValidatorTest, ValidateNotEmpty_Success)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const auto result = ImageValidator::validateNotEmpty(processor);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации на пустоту - невалидное изображение
TEST_F(ImageValidatorTest, ValidateNotEmpty_InvalidImage)
{
    ImageProcessor processor;
    // Изображение не загружено

    const auto result = ImageValidator::validateNotEmpty(processor);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест валидации значений пикселей - успешный случай (выборочная проверка)
TEST_F(ImageValidatorTest, ValidatePixelValues_Success)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const auto result = ImageValidator::validatePixelValues(processor, 0.1);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации значений пикселей - полная проверка
TEST_F(ImageValidatorTest, ValidatePixelValues_FullCheck)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const auto result = ImageValidator::validatePixelValues(processor, 1.0);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации значений пикселей - невалидное изображение
TEST_F(ImageValidatorTest, ValidatePixelValues_InvalidImage)
{
    ImageProcessor processor;
    // Изображение не загружено

    const auto result = ImageValidator::validatePixelValues(processor, 0.1);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест валидации целостности данных - успешный случай (без глубокой проверки)
TEST_F(ImageValidatorTest, ValidateDataIntegrity_Success)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const auto result = ImageValidator::validateDataIntegrity(processor, false);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации целостности данных - успешный случай (с глубокой проверкой)
TEST_F(ImageValidatorTest, ValidateDataIntegrity_DeepCheck)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const auto result = ImageValidator::validateDataIntegrity(processor, true);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации целостности данных - невалидное изображение
TEST_F(ImageValidatorTest, ValidateDataIntegrity_InvalidImage)
{
    ImageProcessor processor;
    // Изображение не загружено

    const auto result = ImageValidator::validateDataIntegrity(processor, false);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест полной валидации - успешный случай (без глубокой проверки)
TEST_F(ImageValidatorTest, ValidateFull_Success)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const auto result = ImageValidator::validateFull(processor, false);
    EXPECT_TRUE(result.isSuccess());
}

// Тест полной валидации - успешный случай (с глубокой проверкой)
TEST_F(ImageValidatorTest, ValidateFull_DeepCheck)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const auto result = ImageValidator::validateFull(processor, true);
    EXPECT_TRUE(result.isSuccess());
}

// Тест полной валидации - невалидное изображение
TEST_F(ImageValidatorTest, ValidateFull_InvalidImage)
{
    ImageProcessor processor;
    // Изображение не загружено

    const auto result = ImageValidator::validateFull(processor, false);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

// Тест валидации с RGBA изображением
TEST_F(ImageValidatorTest, ValidateBasic_RGBA)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto rgba_image = TestUtils::createTestImageRGBA(width, height);
    const auto rgba_file = test_dir_ / "test_rgba.png";
    const auto saved_file = TestUtils::saveTestImagePNG(rgba_image, width, height, rgba_file.string());
    ASSERT_FALSE(saved_file.empty());

    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(saved_file, true).isSuccess());

    const auto result = ImageValidator::validateBasic(processor);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации с большим изображением
TEST_F(ImageValidatorTest, ValidateBasic_LargeImage)
{
    constexpr int width = 100;
    constexpr int height = 100;
    auto large_image = TestUtils::createTestImage(width, height);
    const auto large_file = test_dir_ / "test_large.png";
    const auto saved_file = TestUtils::saveTestImagePNG(large_image, width, height, large_file.string());
    ASSERT_FALSE(saved_file.empty());

    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(saved_file).isSuccess());

    const auto result = ImageValidator::validateFull(processor, false);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации значений пикселей с различными sample_rate
TEST_F(ImageValidatorTest, ValidatePixelValues_VariousSampleRates)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    // Тест с различными значениями sample_rate
    EXPECT_TRUE(ImageValidator::validatePixelValues(processor, 0.0).isSuccess());
    EXPECT_TRUE(ImageValidator::validatePixelValues(processor, 0.1).isSuccess());
    EXPECT_TRUE(ImageValidator::validatePixelValues(processor, 0.5).isSuccess());
    EXPECT_TRUE(ImageValidator::validatePixelValues(processor, 1.0).isSuccess());
}

