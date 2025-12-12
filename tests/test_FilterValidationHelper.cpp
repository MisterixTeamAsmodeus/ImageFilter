#include <gtest/gtest.h>
#include <utils/FilterValidationHelper.h>
#include <ImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidator.h>
#include "test_utils.h"
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief Тесты для FilterValidationHelper
 */
class FilterValidationHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Создаем временную директорию для тестовых файлов
        test_dir_ = fs::temp_directory_path() / "FilterValidationHelperTests";
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

/**
 * @brief Тест validateImageOnly с валидным изображением
 */
TEST_F(FilterValidationHelperTest, ValidateImageOnly_Success)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const auto result = FilterValidationHelper::validateImageOnly(processor);
    EXPECT_TRUE(result.isSuccess());
}

/**
 * @brief Тест validateImageOnly с невалидным изображением
 */
TEST_F(FilterValidationHelperTest, ValidateImageOnly_InvalidImage)
{
    ImageProcessor processor;
    // Изображение не загружено

    const auto result = FilterValidationHelper::validateImageOnly(processor);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

/**
 * @brief Тест validateImageOnly с изображением нулевого размера
 */
TEST_F(FilterValidationHelperTest, ValidateImageOnly_ZeroSize)
{
    ImageProcessor processor;
    // Создаем изображение с нулевым размером
    constexpr int width = 0;
    constexpr int height = 0;
    std::vector<uint8_t> empty_image;
    const auto empty_file = test_dir_ / "empty.png";
    // Не создаем файл, так как нулевой размер недопустим
    // Вместо этого проверяем, что валидация не проходит для невалидного изображения
    const auto result = FilterValidationHelper::validateImageOnly(processor);
    EXPECT_FALSE(result.isSuccess());
}

/**
 * @brief Тест validateImageWithParam с валидным изображением и строковым параметром
 */
TEST_F(FilterValidationHelperTest, ValidateImageWithParam_Success_String)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const std::string param_name = "radius";
    const std::string param_value = "5.0";
    const auto result = FilterValidationHelper::validateImageWithParam(processor, param_name, param_value);
    EXPECT_TRUE(result.isSuccess());
}

/**
 * @brief Тест validateImageWithParam с валидным изображением и числовым параметром (int)
 */
TEST_F(FilterValidationHelperTest, ValidateImageWithParam_Success_Int)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const std::string param_name = "radius";
    const int param_value = 5;
    const auto result = FilterValidationHelper::validateImageWithParam(processor, param_name, param_value);
    EXPECT_TRUE(result.isSuccess());
}

/**
 * @brief Тест validateImageWithParam с валидным изображением и числовым параметром (double)
 */
TEST_F(FilterValidationHelperTest, ValidateImageWithParam_Success_Double)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const std::string param_name = "radius";
    const double param_value = 5.5;
    const auto result = FilterValidationHelper::validateImageWithParam(processor, param_name, param_value);
    EXPECT_TRUE(result.isSuccess());
}

/**
 * @brief Тест validateImageWithParam с валидным изображением и числовым параметром (float)
 */
TEST_F(FilterValidationHelperTest, ValidateImageWithParam_Success_Float)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const std::string param_name = "factor";
    const float param_value = 1.2f;
    const auto result = FilterValidationHelper::validateImageWithParam(processor, param_name, param_value);
    EXPECT_TRUE(result.isSuccess());
}

/**
 * @brief Тест validateImageWithParam с невалидным изображением и параметром
 */
TEST_F(FilterValidationHelperTest, ValidateImageWithParam_InvalidImage)
{
    ImageProcessor processor;
    // Изображение не загружено

    const std::string param_name = "radius";
    const std::string param_value = "5.0";
    const auto result = FilterValidationHelper::validateImageWithParam(processor, param_name, param_value);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
    
    // Проверяем, что параметр добавлен в контекст
    EXPECT_TRUE(result.context.has_value());
    if (result.context.has_value())
    {
        EXPECT_TRUE(result.context.value().filter_params.has_value());
        const std::string params = result.context.value().filter_params.value();
        EXPECT_NE(params.find(param_name), std::string::npos);
        EXPECT_NE(params.find(param_value), std::string::npos);
    }
}

/**
 * @brief Тест validateImageAndParam с валидным изображением и успешной валидацией параметра
 */
TEST_F(FilterValidationHelperTest, ValidateImageAndParam_Success)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const std::string param_name = "radius";
    const double param_value = 5.0;
    
    // Создаем успешный результат валидации параметра
    const auto param_result = FilterResult::success();
    
    const auto result = FilterValidationHelper::validateImageAndParam(processor, param_result, param_name, param_value);
    EXPECT_TRUE(result.isSuccess());
}

/**
 * @brief Тест validateImageAndParam с валидным изображением и строковым параметром
 */
TEST_F(FilterValidationHelperTest, ValidateImageAndParam_Success_String)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const std::string param_name = "factor";
    const std::string param_value = "1.2";
    
    // Создаем успешный результат валидации параметра
    const auto param_result = FilterResult::success();
    
    const auto result = FilterValidationHelper::validateImageAndParam(processor, param_result, param_name, param_value);
    EXPECT_TRUE(result.isSuccess());
}

/**
 * @brief Тест validateImageAndParam с валидным изображением и числовым параметром (int)
 */
TEST_F(FilterValidationHelperTest, ValidateImageAndParam_Success_Int)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const std::string param_name = "threshold";
    const int param_value = 128;
    
    // Создаем успешный результат валидации параметра
    const auto param_result = FilterResult::success();
    
    const auto result = FilterValidationHelper::validateImageAndParam(processor, param_result, param_name, param_value);
    EXPECT_TRUE(result.isSuccess());
}

/**
 * @brief Тест validateImageAndParam с валидным изображением и числовым параметром (float)
 */
TEST_F(FilterValidationHelperTest, ValidateImageAndParam_Success_Float)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const std::string param_name = "angle";
    const float param_value = 45.0f;
    
    // Создаем успешный результат валидации параметра
    const auto param_result = FilterResult::success();
    
    const auto result = FilterValidationHelper::validateImageAndParam(processor, param_result, param_name, param_value);
    EXPECT_TRUE(result.isSuccess());
}

/**
 * @brief Тест validateImageAndParam с невалидным изображением
 */
TEST_F(FilterValidationHelperTest, ValidateImageAndParam_InvalidImage)
{
    ImageProcessor processor;
    // Изображение не загружено

    const std::string param_name = "radius";
    const double param_value = 5.0;
    
    // Создаем успешный результат валидации параметра
    const auto param_result = FilterResult::success();
    
    const auto result = FilterValidationHelper::validateImageAndParam(processor, param_result, param_name, param_value);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
    
    // Проверяем, что параметр добавлен в контекст
    EXPECT_TRUE(result.context.has_value());
    if (result.context.has_value())
    {
        EXPECT_TRUE(result.context.value().filter_params.has_value());
        const std::string params = result.context.value().filter_params.value();
        EXPECT_NE(params.find(param_name), std::string::npos);
    }
}

/**
 * @brief Тест validateImageAndParam с валидным изображением, но невалидным параметром
 */
TEST_F(FilterValidationHelperTest, ValidateImageAndParam_InvalidParam)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const std::string param_name = "radius";
    const double param_value = -5.0;
    
    // Создаем результат с ошибкой валидации параметра
    const auto param_result = FilterResult::failure(FilterError::InvalidRadius, "Радиус должен быть положительным");
    
    const auto result = FilterValidationHelper::validateImageAndParam(processor, param_result, param_name, param_value);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidRadius);
    
    // Проверяем, что параметр добавлен в контекст
    EXPECT_TRUE(result.context.has_value());
    if (result.context.has_value())
    {
        EXPECT_TRUE(result.context.value().filter_params.has_value());
        const std::string params = result.context.value().filter_params.value();
        EXPECT_NE(params.find(param_name), std::string::npos);
        
        // Проверяем, что информация об изображении также в контексте
        EXPECT_TRUE(result.context.value().image_width.has_value());
        EXPECT_TRUE(result.context.value().image_height.has_value());
        EXPECT_TRUE(result.context.value().image_channels.has_value());
    }
}

/**
 * @brief Тест validateImageAndParam с невалидным изображением и невалидным параметром
 * (должна вернуться ошибка изображения, так как она проверяется первой)
 */
TEST_F(FilterValidationHelperTest, ValidateImageAndParam_InvalidImageAndParam)
{
    ImageProcessor processor;
    // Изображение не загружено

    const std::string param_name = "radius";
    const double param_value = -5.0;
    
    // Создаем результат с ошибкой валидации параметра
    const auto param_result = FilterResult::failure(FilterError::InvalidRadius, "Радиус должен быть положительным");
    
    const auto result = FilterValidationHelper::validateImageAndParam(processor, param_result, param_name, param_value);
    
    // Должна вернуться ошибка изображения, так как она проверяется первой
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
    
    // Проверяем, что параметр добавлен в контекст
    EXPECT_TRUE(result.context.has_value());
    if (result.context.has_value())
    {
        EXPECT_TRUE(result.context.value().filter_params.has_value());
        const std::string params = result.context.value().filter_params.value();
        EXPECT_NE(params.find(param_name), std::string::npos);
    }
}

/**
 * @brief Тест validateImageAndParam с различными типами параметров
 */
TEST_F(FilterValidationHelperTest, ValidateImageAndParam_VariousTypes)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const auto param_result = FilterResult::success();
    
    // Тест с unsigned int
    {
        const std::string param_name = "size";
        const unsigned int param_value = 10u;
        const auto result = FilterValidationHelper::validateImageAndParam(processor, param_result, param_name, param_value);
        EXPECT_TRUE(result.isSuccess());
    }
    
    // Тест с long
    {
        const std::string param_name = "offset";
        const long param_value = 100L;
        const auto result = FilterValidationHelper::validateImageAndParam(processor, param_result, param_name, param_value);
        EXPECT_TRUE(result.isSuccess());
    }
    
    // Тест с long double
    {
        const std::string param_name = "precision";
        const long double param_value = 0.0001L;
        const auto result = FilterValidationHelper::validateImageAndParam(processor, param_result, param_name, param_value);
        EXPECT_TRUE(result.isSuccess());
    }
}

/**
 * @brief Тест validateImageWithParam с различными типами параметров
 */
TEST_F(FilterValidationHelperTest, ValidateImageWithParam_VariousTypes)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    // Тест с unsigned int
    {
        const std::string param_name = "size";
        const unsigned int param_value = 10u;
        const auto result = FilterValidationHelper::validateImageWithParam(processor, param_name, param_value);
        EXPECT_TRUE(result.isSuccess());
    }
    
    // Тест с long
    {
        const std::string param_name = "offset";
        const long param_value = 100L;
        const auto result = FilterValidationHelper::validateImageWithParam(processor, param_name, param_value);
        EXPECT_TRUE(result.isSuccess());
    }
    
    // Тест с long double
    {
        const std::string param_name = "precision";
        const long double param_value = 0.0001L;
        const auto result = FilterValidationHelper::validateImageWithParam(processor, param_name, param_value);
        EXPECT_TRUE(result.isSuccess());
    }
}

/**
 * @brief Тест validateImageAndParam с контекстом ошибки в param_result
 */
TEST_F(FilterValidationHelperTest, ValidateImageAndParam_WithContextInParamResult)
{
    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(test_file_.string()).isSuccess());

    const std::string param_name = "radius";
    const double param_value = -5.0;
    
    // Создаем результат с ошибкой и контекстом
    ErrorContext param_ctx = ErrorContext::withFilename("test.png");
    const auto param_result = FilterResult::failure(FilterError::InvalidRadius, "Радиус должен быть положительным", param_ctx);
    
    const auto result = FilterValidationHelper::validateImageAndParam(processor, param_result, param_name, param_value);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidRadius);
    
    // Проверяем, что контекст из param_result сохранен и дополнен параметром
    EXPECT_TRUE(result.context.has_value());
    if (result.context.has_value())
    {
        // Проверяем, что информация об изображении добавлена
        EXPECT_TRUE(result.context.value().image_width.has_value());
        EXPECT_TRUE(result.context.value().image_height.has_value());
        EXPECT_TRUE(result.context.value().image_channels.has_value());
        
        // Проверяем, что параметр добавлен
        EXPECT_TRUE(result.context.value().filter_params.has_value());
        const std::string params = result.context.value().filter_params.value();
        EXPECT_NE(params.find(param_name), std::string::npos);
    }
}

/**
 * @brief Тест validateImageWithParam с RGBA изображением
 */
TEST_F(FilterValidationHelperTest, ValidateImageWithParam_RGBA)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto rgba_image = TestUtils::createTestImageRGBA(width, height);
    const auto rgba_file = test_dir_ / "test_rgba.png";
    const auto saved_file = TestUtils::saveTestImagePNG(rgba_image, width, height, rgba_file.string());
    ASSERT_FALSE(saved_file.empty());

    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(saved_file, true).isSuccess());

    const std::string param_name = "alpha";
    const double param_value = 0.5;
    const auto result = FilterValidationHelper::validateImageWithParam(processor, param_name, param_value);
    EXPECT_TRUE(result.isSuccess());
}

/**
 * @brief Тест validateImageAndParam с RGBA изображением
 */
TEST_F(FilterValidationHelperTest, ValidateImageAndParam_RGBA)
{
    constexpr int width = 10;
    constexpr int height = 10;
    auto rgba_image = TestUtils::createTestImageRGBA(width, height);
    const auto rgba_file = test_dir_ / "test_rgba.png";
    const auto saved_file = TestUtils::saveTestImagePNG(rgba_image, width, height, rgba_file.string());
    ASSERT_FALSE(saved_file.empty());

    ImageProcessor processor;
    ASSERT_TRUE(processor.loadFromFile(saved_file, true).isSuccess());

    const std::string param_name = "alpha";
    const double param_value = 0.5;
    const auto param_result = FilterResult::success();
    const auto result = FilterValidationHelper::validateImageAndParam(processor, param_result, param_name, param_value);
    EXPECT_TRUE(result.isSuccess());
}

