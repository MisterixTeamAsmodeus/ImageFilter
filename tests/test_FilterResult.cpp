#include "test_utils.h"
#include <utils/FilterResult.h>
#include <utils/ErrorCodes.h>
#include <gtest/gtest.h>
#include <cerrno>

/**
 * @brief Тесты для FilterResult и filterErrorToString
 */
class FilterResultTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Тест успешного результата
 */
TEST_F(FilterResultTest, FilterErrorToString_Success)
{
    const std::string result = filterErrorToString(FilterError::Success);
    EXPECT_EQ(result, "Успешно");
}

// Тесты для ошибок валидации (1-99)
TEST_F(FilterResultTest, FilterErrorToString_InvalidImage)
{
    const std::string result = filterErrorToString(FilterError::InvalidImage);
    EXPECT_EQ(result, "Невалидное изображение");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidChannels)
{
    const std::string result = filterErrorToString(FilterError::InvalidChannels);
    EXPECT_EQ(result, "Невалидное количество каналов");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidParameter)
{
    const std::string result = filterErrorToString(FilterError::InvalidParameter);
    EXPECT_EQ(result, "Невалидный параметр");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidWidth)
{
    const std::string result = filterErrorToString(FilterError::InvalidWidth);
    EXPECT_EQ(result, "Невалидная ширина изображения");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidHeight)
{
    const std::string result = filterErrorToString(FilterError::InvalidHeight);
    EXPECT_EQ(result, "Невалидная высота изображения");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidSize)
{
    const std::string result = filterErrorToString(FilterError::InvalidSize);
    EXPECT_EQ(result, "Невалидный размер изображения");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidRadius)
{
    const std::string result = filterErrorToString(FilterError::InvalidRadius);
    EXPECT_EQ(result, "Невалидный радиус (должен быть > 0)");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidFactor)
{
    const std::string result = filterErrorToString(FilterError::InvalidFactor);
    EXPECT_EQ(result, "Невалидный коэффициент (должен быть > 0)");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidThreshold)
{
    const std::string result = filterErrorToString(FilterError::InvalidThreshold);
    EXPECT_EQ(result, "Невалидное пороговое значение");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidQuality)
{
    const std::string result = filterErrorToString(FilterError::InvalidQuality);
    EXPECT_EQ(result, "Невалидное качество (должно быть в диапазоне 0-100)");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidAngle)
{
    const std::string result = filterErrorToString(FilterError::InvalidAngle);
    EXPECT_EQ(result, "Невалидный угол поворота");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidKernelSize)
{
    const std::string result = filterErrorToString(FilterError::InvalidKernelSize);
    EXPECT_EQ(result, "Невалидный размер ядра свертки");
}

TEST_F(FilterResultTest, FilterErrorToString_ParameterOutOfRange)
{
    const std::string result = filterErrorToString(FilterError::ParameterOutOfRange);
    EXPECT_EQ(result, "Параметр вне допустимого диапазона");
}

TEST_F(FilterResultTest, FilterErrorToString_EmptyImage)
{
    const std::string result = filterErrorToString(FilterError::EmptyImage);
    EXPECT_EQ(result, "Изображение пустое (нулевой размер)");
}

// Тесты для ошибок памяти (100-199)
TEST_F(FilterResultTest, FilterErrorToString_OutOfMemory)
{
    const std::string result = filterErrorToString(FilterError::OutOfMemory);
    EXPECT_EQ(result, "Недостаточно памяти");
}

TEST_F(FilterResultTest, FilterErrorToString_MemoryAllocationFailed)
{
    const std::string result = filterErrorToString(FilterError::MemoryAllocationFailed);
    EXPECT_EQ(result, "Ошибка выделения памяти");
}

TEST_F(FilterResultTest, FilterErrorToString_MemoryOverflow)
{
    const std::string result = filterErrorToString(FilterError::MemoryOverflow);
    EXPECT_EQ(result, "Переполнение памяти при вычислениях");
}

// Тесты для ошибок формата (200-299)
TEST_F(FilterResultTest, FilterErrorToString_UnsupportedFormat)
{
    const std::string result = filterErrorToString(FilterError::UnsupportedFormat);
    EXPECT_EQ(result, "Неподдерживаемый формат изображения");
}

TEST_F(FilterResultTest, FilterErrorToString_UnsupportedChannels)
{
    const std::string result = filterErrorToString(FilterError::UnsupportedChannels);
    EXPECT_EQ(result, "Неподдерживаемое количество каналов");
}

TEST_F(FilterResultTest, FilterErrorToString_CorruptedImage)
{
    const std::string result = filterErrorToString(FilterError::CorruptedImage);
    EXPECT_EQ(result, "Поврежденное изображение");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidImageData)
{
    const std::string result = filterErrorToString(FilterError::InvalidImageData);
    EXPECT_EQ(result, "Некорректные данные изображения");
}

TEST_F(FilterResultTest, FilterErrorToString_FormatMismatch)
{
    const std::string result = filterErrorToString(FilterError::FormatMismatch);
    EXPECT_EQ(result, "Несоответствие формата ожидаемому");
}

// Тесты для ошибок вычислений (300-399)
TEST_F(FilterResultTest, FilterErrorToString_DivisionByZero)
{
    const std::string result = filterErrorToString(FilterError::DivisionByZero);
    EXPECT_EQ(result, "Деление на ноль");
}

TEST_F(FilterResultTest, FilterErrorToString_ArithmeticOverflow)
{
    const std::string result = filterErrorToString(FilterError::ArithmeticOverflow);
    EXPECT_EQ(result, "Арифметическое переполнение");
}

TEST_F(FilterResultTest, FilterErrorToString_ArithmeticUnderflow)
{
    const std::string result = filterErrorToString(FilterError::ArithmeticUnderflow);
    EXPECT_EQ(result, "Арифметическое исчерпание");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidKernel)
{
    const std::string result = filterErrorToString(FilterError::InvalidKernel);
    EXPECT_EQ(result, "Некорректное ядро свертки");
}

TEST_F(FilterResultTest, FilterErrorToString_KernelNormalizationFailed)
{
    const std::string result = filterErrorToString(FilterError::KernelNormalizationFailed);
    EXPECT_EQ(result, "Ошибка нормализации ядра");
}

// Тесты для ошибок файловых операций (400-499)
TEST_F(FilterResultTest, FilterErrorToString_FileOperationFailed)
{
    const std::string result = filterErrorToString(FilterError::FileOperationFailed);
    EXPECT_EQ(result, "Ошибка операции с файлом");
}

TEST_F(FilterResultTest, FilterErrorToString_FileNotFound)
{
    const std::string result = filterErrorToString(FilterError::FileNotFound);
    EXPECT_EQ(result, "Файл не найден");
}

TEST_F(FilterResultTest, FilterErrorToString_FileReadError)
{
    const std::string result = filterErrorToString(FilterError::FileReadError);
    EXPECT_EQ(result, "Ошибка чтения файла");
}

TEST_F(FilterResultTest, FilterErrorToString_FileWriteError)
{
    const std::string result = filterErrorToString(FilterError::FileWriteError);
    EXPECT_EQ(result, "Ошибка записи файла");
}

TEST_F(FilterResultTest, FilterErrorToString_FilePermissionDenied)
{
    const std::string result = filterErrorToString(FilterError::FilePermissionDenied);
    EXPECT_EQ(result, "Отказано в доступе к файлу");
}

TEST_F(FilterResultTest, FilterErrorToString_FileTooLarge)
{
    const std::string result = filterErrorToString(FilterError::FileTooLarge);
    EXPECT_EQ(result, "Файл слишком большой");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidFilePath)
{
    const std::string result = filterErrorToString(FilterError::InvalidFilePath);
    EXPECT_EQ(result, "Некорректный путь к файлу");
}

// Тесты для системных ошибок (500-599)
TEST_F(FilterResultTest, FilterErrorToString_SystemError)
{
    const std::string result = filterErrorToString(FilterError::SystemError);
    EXPECT_EQ(result, "Системная ошибка");
}

TEST_F(FilterResultTest, FilterErrorToString_InvalidSystemCall)
{
    const std::string result = filterErrorToString(FilterError::InvalidSystemCall);
    EXPECT_EQ(result, "Некорректный системный вызов");
}

/**
 * @brief Тест FilterResult::success()
 */
TEST_F(FilterResultTest, FilterResult_Success)
{
    const auto result = FilterResult::success();
    EXPECT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.hasError());
    EXPECT_EQ(result.error, FilterError::Success);
    EXPECT_TRUE(result.message.empty());
    EXPECT_FALSE(result.context.has_value());
}

/**
 * @brief Тест FilterResult::failure()
 */
TEST_F(FilterResultTest, FilterResult_Failure)
{
    const std::string error_msg = "Тестовая ошибка";
    const auto result = FilterResult::failure(FilterError::InvalidImage, error_msg);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.hasError());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
    EXPECT_EQ(result.message, error_msg);
    EXPECT_FALSE(result.context.has_value());
}

/**
 * @brief Тест FilterResult::failure() с контекстом
 */
TEST_F(FilterResultTest, FilterResult_FailureWithContext)
{
    const std::string error_msg = "Тестовая ошибка";
    ErrorContext ctx = ErrorContext::withFilename("test.png");
    const auto result = FilterResult::failure(FilterError::FileReadError, error_msg, ctx);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.hasError());
    EXPECT_EQ(result.error, FilterError::FileReadError);
    EXPECT_EQ(result.message, error_msg);
    EXPECT_TRUE(result.context.has_value());
    EXPECT_TRUE(result.context.value().filename.has_value());
    EXPECT_EQ(result.context.value().filename.value(), "test.png");
}

/**
 * @brief Тест FilterResult::failureWithImage()
 */
TEST_F(FilterResultTest, FilterResult_FailureWithImage)
{
    const std::string error_msg = "Ошибка изображения";
    const auto result = FilterResult::failureWithImage(FilterError::InvalidSize, error_msg, 100, 200, 3);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.hasError());
    EXPECT_EQ(result.error, FilterError::InvalidSize);
    EXPECT_EQ(result.message, error_msg);
    EXPECT_TRUE(result.context.has_value());
    EXPECT_TRUE(result.context.value().image_width.has_value());
    EXPECT_EQ(result.context.value().image_width.value(), 100);
    EXPECT_TRUE(result.context.value().image_height.has_value());
    EXPECT_EQ(result.context.value().image_height.value(), 200);
    EXPECT_TRUE(result.context.value().image_channels.has_value());
    EXPECT_EQ(result.context.value().image_channels.value(), 3);
}

/**
 * @brief Тест FilterResult::failureWithFilename()
 */
TEST_F(FilterResultTest, FilterResult_FailureWithFilename)
{
    const std::string error_msg = "Ошибка файла";
    const std::string filename = "test_image.png";
    const auto result = FilterResult::failureWithFilename(FilterError::FileNotFound, error_msg, filename);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.hasError());
    EXPECT_EQ(result.error, FilterError::FileNotFound);
    EXPECT_EQ(result.message, error_msg);
    EXPECT_TRUE(result.context.has_value());
    EXPECT_TRUE(result.context.value().filename.has_value());
    EXPECT_EQ(result.context.value().filename.value(), filename);
}

/**
 * @brief Тест FilterResult::failureWithSystemError()
 */
TEST_F(FilterResultTest, FilterResult_FailureWithSystemError)
{
    const std::string error_msg = "Системная ошибка";
    const int errno_code = ENOENT;
    const auto result = FilterResult::failureWithSystemError(FilterError::SystemError, error_msg, errno_code);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.hasError());
    EXPECT_EQ(result.error, FilterError::SystemError);
    EXPECT_EQ(result.message, error_msg);
    EXPECT_TRUE(result.context.has_value());
    EXPECT_TRUE(result.context.value().system_error_code.has_value());
    EXPECT_EQ(result.context.value().system_error_code.value(), errno_code);
}

/**
 * @brief Тест FilterResult::getFullMessage() для успешного результата
 */
TEST_F(FilterResultTest, FilterResult_GetFullMessage_Success)
{
    const auto result = FilterResult::success();
    const std::string full_msg = result.getFullMessage();
    EXPECT_EQ(full_msg, "Успешно");
}

/**
 * @brief Тест FilterResult::getFullMessage() с сообщением
 */
TEST_F(FilterResultTest, FilterResult_GetFullMessage_WithMessage)
{
    const std::string error_msg = "Тестовая ошибка";
    const auto result = FilterResult::failure(FilterError::InvalidImage, error_msg);
    const std::string full_msg = result.getFullMessage();
    EXPECT_EQ(full_msg, error_msg);
}

/**
 * @brief Тест FilterResult::getFullMessage() с контекстом
 */
TEST_F(FilterResultTest, FilterResult_GetFullMessage_WithContext)
{
    ErrorContext ctx = ErrorContext::withFilename("test.png");
    const auto result = FilterResult::failure(FilterError::FileReadError, "", ctx);
    const std::string full_msg = result.getFullMessage();
    EXPECT_FALSE(full_msg.empty());
    EXPECT_NE(full_msg.find("test.png"), std::string::npos);
}

/**
 * @brief Тест FilterResult::getFullMessage() с сообщением и контекстом
 */
TEST_F(FilterResultTest, FilterResult_GetFullMessage_WithMessageAndContext)
{
    const std::string error_msg = "Ошибка загрузки";
    ErrorContext ctx = ErrorContext::withFilename("test.png");
    const auto result = FilterResult::failure(FilterError::FileReadError, error_msg, ctx);
    const std::string full_msg = result.getFullMessage();
    EXPECT_FALSE(full_msg.empty());
    EXPECT_NE(full_msg.find(error_msg), std::string::npos);
    EXPECT_NE(full_msg.find("test.png"), std::string::npos);
}

/**
 * @brief Тест ErrorContext::withImage()
 */
TEST_F(FilterResultTest, ErrorContext_WithImage)
{
    const auto ctx = ErrorContext::withImage(100, 200, 3);
    EXPECT_TRUE(ctx.image_width.has_value());
    EXPECT_EQ(ctx.image_width.value(), 100);
    EXPECT_TRUE(ctx.image_height.has_value());
    EXPECT_EQ(ctx.image_height.value(), 200);
    EXPECT_TRUE(ctx.image_channels.has_value());
    EXPECT_EQ(ctx.image_channels.value(), 3);
}

/**
 * @brief Тест ErrorContext::withFilename()
 */
TEST_F(FilterResultTest, ErrorContext_WithFilename)
{
    const std::string filename = "test.png";
    const auto ctx = ErrorContext::withFilename(filename);
    EXPECT_TRUE(ctx.filename.has_value());
    EXPECT_EQ(ctx.filename.value(), filename);
}

/**
 * @brief Тест ErrorContext::withSystemError()
 */
TEST_F(FilterResultTest, ErrorContext_WithSystemError)
{
    const int errno_code = ENOENT;
    const auto ctx = ErrorContext::withSystemError(errno_code);
    EXPECT_TRUE(ctx.system_error_code.has_value());
    EXPECT_EQ(ctx.system_error_code.value(), errno_code);
}

/**
 * @brief Тест ErrorContext::withFilterParam() (строковый)
 */
TEST_F(FilterResultTest, ErrorContext_WithFilterParam_String)
{
    ErrorContext ctx;
    ctx.withFilterParam("radius", "5.0");
    EXPECT_TRUE(ctx.filter_params.has_value());
    EXPECT_NE(ctx.filter_params.value().find("radius=5.0"), std::string::npos);
}

/**
 * @brief Тест ErrorContext::withFilterParam() (числовой)
 */
TEST_F(FilterResultTest, ErrorContext_WithFilterParam_Numeric)
{
    ErrorContext ctx;
    ctx.withFilterParam("radius", 5.0);
    EXPECT_TRUE(ctx.filter_params.has_value());
    EXPECT_NE(ctx.filter_params.value().find("radius="), std::string::npos);
}

/**
 * @brief Тест ErrorContext::withFilterParam() (несколько параметров)
 */
TEST_F(FilterResultTest, ErrorContext_WithFilterParam_Multiple)
{
    ErrorContext ctx;
    ctx.withFilterParam("radius", "5.0")
       .withFilterParam("factor", "1.2");
    EXPECT_TRUE(ctx.filter_params.has_value());
    EXPECT_NE(ctx.filter_params.value().find("radius=5.0"), std::string::npos);
    EXPECT_NE(ctx.filter_params.value().find("factor=1.2"), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с файлом
 */
TEST_F(FilterResultTest, ErrorContext_ToString_WithFilename)
{
    const auto ctx = ErrorContext::withFilename("test.png");
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("test.png"), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с изображением
 */
TEST_F(FilterResultTest, ErrorContext_ToString_WithImage)
{
    const auto ctx = ErrorContext::withImage(100, 200, 3);
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("100"), std::string::npos);
    EXPECT_NE(str.find("200"), std::string::npos);
    EXPECT_NE(str.find("3"), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с параметрами фильтра
 */
TEST_F(FilterResultTest, ErrorContext_ToString_WithFilterParams)
{
    ErrorContext ctx;
    ctx.withFilterParam("radius", "5.0");
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("radius=5.0"), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с системной ошибкой
 */
TEST_F(FilterResultTest, ErrorContext_ToString_WithSystemError)
{
    const auto ctx = ErrorContext::withSystemError(ENOENT);
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find(std::to_string(ENOENT)), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() со всеми полями
 */
TEST_F(FilterResultTest, ErrorContext_ToString_AllFields)
{
    ErrorContext ctx = ErrorContext::withFilename("test.png");
    ctx.image_width = 100;
    ctx.image_height = 200;
    ctx.image_channels = 3;
    ctx.withFilterParam("radius", "5.0");
    ctx.system_error_code = ENOENT;
    
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("test.png"), std::string::npos);
    EXPECT_NE(str.find("100"), std::string::npos);
    EXPECT_NE(str.find("200"), std::string::npos);
    EXPECT_NE(str.find("3"), std::string::npos);
    EXPECT_NE(str.find("radius=5.0"), std::string::npos);
    EXPECT_NE(str.find(std::to_string(ENOENT)), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с пустым контекстом
 */
TEST_F(FilterResultTest, ErrorContext_ToString_Empty)
{
    ErrorContext ctx;
    const std::string str = ctx.toString();
    EXPECT_TRUE(str.empty());
}

/**
 * @brief Тест ErrorContext::withFilterParam() с цепочкой вызовов (разные типы)
 */
TEST_F(FilterResultTest, ErrorContext_WithFilterParam_ChainDifferentTypes)
{
    ErrorContext ctx;
    ctx.withFilterParam("radius", 5)
       .withFilterParam("factor", 1.2)
       .withFilterParam("threshold", "128")
       .withFilterParam("enabled", true);
    
    EXPECT_TRUE(ctx.filter_params.has_value());
    const std::string params = ctx.filter_params.value();
    EXPECT_NE(params.find("radius="), std::string::npos);
    EXPECT_NE(params.find("factor="), std::string::npos);
    EXPECT_NE(params.find("threshold=128"), std::string::npos);
    EXPECT_NE(params.find("enabled="), std::string::npos);
}

/**
 * @brief Тест ErrorContext::withFilterParam() с большим количеством параметров
 */
TEST_F(FilterResultTest, ErrorContext_WithFilterParam_ManyParams)
{
    ErrorContext ctx;
    ctx.withFilterParam("param1", "value1")
       .withFilterParam("param2", "value2")
       .withFilterParam("param3", "value3")
       .withFilterParam("param4", "value4")
       .withFilterParam("param5", "value5");
    
    EXPECT_TRUE(ctx.filter_params.has_value());
    const std::string params = ctx.filter_params.value();
    EXPECT_NE(params.find("param1=value1"), std::string::npos);
    EXPECT_NE(params.find("param5=value5"), std::string::npos);
    // Проверяем, что параметры разделены запятыми
    EXPECT_NE(params.find(", "), std::string::npos);
}

/**
 * @brief Тест ErrorContext::withFilterParam() с различными числовыми типами
 */
TEST_F(FilterResultTest, ErrorContext_WithFilterParam_VariousNumericTypes)
{
    ErrorContext ctx;
    
    // Тест с int
    ctx.withFilterParam("int_val", 42);
    EXPECT_TRUE(ctx.filter_params.has_value());
    EXPECT_NE(ctx.filter_params.value().find("int_val=42"), std::string::npos);
    
    // Тест с unsigned int
    ErrorContext ctx2;
    ctx2.withFilterParam("uint_val", 42u);
    EXPECT_TRUE(ctx2.filter_params.has_value());
    EXPECT_NE(ctx2.filter_params.value().find("uint_val=42"), std::string::npos);
    
    // Тест с long
    ErrorContext ctx3;
    ctx3.withFilterParam("long_val", 100L);
    EXPECT_TRUE(ctx3.filter_params.has_value());
    EXPECT_NE(ctx3.filter_params.value().find("long_val=100"), std::string::npos);
    
    // Тест с float
    ErrorContext ctx4;
    ctx4.withFilterParam("float_val", 3.14f);
    EXPECT_TRUE(ctx4.filter_params.has_value());
    EXPECT_NE(ctx4.filter_params.value().find("float_val="), std::string::npos);
    
    // Тест с double
    ErrorContext ctx5;
    ctx5.withFilterParam("double_val", 3.14159);
    EXPECT_TRUE(ctx5.filter_params.has_value());
    EXPECT_NE(ctx5.filter_params.value().find("double_val="), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с комбинацией filename и image
 */
TEST_F(FilterResultTest, ErrorContext_ToString_FilenameAndImage)
{
    ErrorContext ctx = ErrorContext::withFilename("test.png");
    ctx.image_width = 100;
    ctx.image_height = 200;
    ctx.image_channels = 3;
    
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("test.png"), std::string::npos);
    EXPECT_NE(str.find("100"), std::string::npos);
    EXPECT_NE(str.find("200"), std::string::npos);
    EXPECT_NE(str.find("3"), std::string::npos);
    // Проверяем, что поля разделены запятыми
    EXPECT_NE(str.find(", "), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с комбинацией filename и filter_params
 */
TEST_F(FilterResultTest, ErrorContext_ToString_FilenameAndFilterParams)
{
    ErrorContext ctx = ErrorContext::withFilename("test.png");
    ctx.withFilterParam("radius", "5.0");
    
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("test.png"), std::string::npos);
    EXPECT_NE(str.find("radius=5.0"), std::string::npos);
    EXPECT_NE(str.find(", "), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с комбинацией filename и system_error
 */
TEST_F(FilterResultTest, ErrorContext_ToString_FilenameAndSystemError)
{
    ErrorContext ctx = ErrorContext::withFilename("test.png");
    ctx.system_error_code = ENOENT;
    
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("test.png"), std::string::npos);
    EXPECT_NE(str.find(std::to_string(ENOENT)), std::string::npos);
    EXPECT_NE(str.find(", "), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с комбинацией image и filter_params
 */
TEST_F(FilterResultTest, ErrorContext_ToString_ImageAndFilterParams)
{
    ErrorContext ctx = ErrorContext::withImage(100, 200, 3);
    ctx.withFilterParam("radius", "5.0");
    
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("100"), std::string::npos);
    EXPECT_NE(str.find("200"), std::string::npos);
    EXPECT_NE(str.find("3"), std::string::npos);
    EXPECT_NE(str.find("radius=5.0"), std::string::npos);
    EXPECT_NE(str.find(", "), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с комбинацией image и system_error
 */
TEST_F(FilterResultTest, ErrorContext_ToString_ImageAndSystemError)
{
    ErrorContext ctx = ErrorContext::withImage(100, 200, 3);
    ctx.system_error_code = EACCES;
    
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("100"), std::string::npos);
    EXPECT_NE(str.find("200"), std::string::npos);
    EXPECT_NE(str.find("3"), std::string::npos);
    EXPECT_NE(str.find(std::to_string(EACCES)), std::string::npos);
    EXPECT_NE(str.find(", "), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с комбинацией filter_params и system_error
 */
TEST_F(FilterResultTest, ErrorContext_ToString_FilterParamsAndSystemError)
{
    ErrorContext ctx;
    ctx.withFilterParam("radius", "5.0");
    ctx.system_error_code = EIO;
    
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("radius=5.0"), std::string::npos);
    EXPECT_NE(str.find(std::to_string(EIO)), std::string::npos);
    EXPECT_NE(str.find(", "), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с filename, image и filter_params
 */
TEST_F(FilterResultTest, ErrorContext_ToString_FilenameImageAndFilterParams)
{
    ErrorContext ctx = ErrorContext::withFilename("test.png");
    ctx.image_width = 100;
    ctx.image_height = 200;
    ctx.image_channels = 3;
    ctx.withFilterParam("radius", "5.0");
    
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("test.png"), std::string::npos);
    EXPECT_NE(str.find("100"), std::string::npos);
    EXPECT_NE(str.find("200"), std::string::npos);
    EXPECT_NE(str.find("3"), std::string::npos);
    EXPECT_NE(str.find("radius=5.0"), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с filename, image и system_error
 */
TEST_F(FilterResultTest, ErrorContext_ToString_FilenameImageAndSystemError)
{
    ErrorContext ctx = ErrorContext::withFilename("test.png");
    ctx.image_width = 100;
    ctx.image_height = 200;
    ctx.image_channels = 3;
    ctx.system_error_code = ENOENT;
    
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("test.png"), std::string::npos);
    EXPECT_NE(str.find("100"), std::string::npos);
    EXPECT_NE(str.find("200"), std::string::npos);
    EXPECT_NE(str.find("3"), std::string::npos);
    EXPECT_NE(str.find(std::to_string(ENOENT)), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с filename, filter_params и system_error
 */
TEST_F(FilterResultTest, ErrorContext_ToString_FilenameFilterParamsAndSystemError)
{
    ErrorContext ctx = ErrorContext::withFilename("test.png");
    ctx.withFilterParam("radius", "5.0");
    ctx.system_error_code = EACCES;
    
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("test.png"), std::string::npos);
    EXPECT_NE(str.find("radius=5.0"), std::string::npos);
    EXPECT_NE(str.find(std::to_string(EACCES)), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с image, filter_params и system_error
 */
TEST_F(FilterResultTest, ErrorContext_ToString_ImageFilterParamsAndSystemError)
{
    ErrorContext ctx = ErrorContext::withImage(100, 200, 3);
    ctx.withFilterParam("radius", "5.0");
    ctx.system_error_code = EIO;
    
    const std::string str = ctx.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("100"), std::string::npos);
    EXPECT_NE(str.find("200"), std::string::npos);
    EXPECT_NE(str.find("3"), std::string::npos);
    EXPECT_NE(str.find("radius=5.0"), std::string::npos);
    EXPECT_NE(str.find(std::to_string(EIO)), std::string::npos);
}

/**
 * @brief Тест ErrorContext::toString() с неполной информацией об изображении (только width)
 */
TEST_F(FilterResultTest, ErrorContext_ToString_PartialImageInfo_WidthOnly)
{
    ErrorContext ctx;
    ctx.image_width = 100;
    // height и channels не установлены
    
    const std::string str = ctx.toString();
    // Если не все поля изображения установлены, информация об изображении не должна отображаться
    EXPECT_TRUE(str.empty());
}

/**
 * @brief Тест ErrorContext::toString() с неполной информацией об изображении (width и height)
 */
TEST_F(FilterResultTest, ErrorContext_ToString_PartialImageInfo_WidthAndHeight)
{
    ErrorContext ctx;
    ctx.image_width = 100;
    ctx.image_height = 200;
    // channels не установлен
    
    const std::string str = ctx.toString();
    // Если не все поля изображения установлены, информация об изображении не должна отображаться
    EXPECT_TRUE(str.empty());
}

/**
 * @brief Тест ErrorContext::toString() с неполной информацией об изображении (только height)
 */
TEST_F(FilterResultTest, ErrorContext_ToString_PartialImageInfo_HeightOnly)
{
    ErrorContext ctx;
    ctx.image_height = 200;
    // width и channels не установлены
    
    const std::string str = ctx.toString();
    // Если не все поля изображения установлены, информация об изображении не должна отображаться
    EXPECT_TRUE(str.empty());
}

/**
 * @brief Тест ErrorContext::toString() с различными системными кодами ошибок
 */
TEST_F(FilterResultTest, ErrorContext_ToString_VariousSystemErrors)
{
    // Тест с ENOENT (файл не найден)
    {
        ErrorContext ctx = ErrorContext::withSystemError(ENOENT);
        const std::string str = ctx.toString();
        EXPECT_FALSE(str.empty());
        EXPECT_NE(str.find(std::to_string(ENOENT)), std::string::npos);
    }
    
    // Тест с EACCES (доступ запрещен)
    {
        ErrorContext ctx = ErrorContext::withSystemError(EACCES);
        const std::string str = ctx.toString();
        EXPECT_FALSE(str.empty());
        EXPECT_NE(str.find(std::to_string(EACCES)), std::string::npos);
    }
    
    // Тест с EIO (ошибка ввода-вывода)
    {
        ErrorContext ctx = ErrorContext::withSystemError(EIO);
        const std::string str = ctx.toString();
        EXPECT_FALSE(str.empty());
        EXPECT_NE(str.find(std::to_string(EIO)), std::string::npos);
    }
    
    // Тест с ENOMEM (недостаточно памяти)
    {
        ErrorContext ctx = ErrorContext::withSystemError(ENOMEM);
        const std::string str = ctx.toString();
        EXPECT_FALSE(str.empty());
        EXPECT_NE(str.find(std::to_string(ENOMEM)), std::string::npos);
    }
}

/**
 * @brief Тест ErrorContext::withFilterParam() с пустыми строками
 */
TEST_F(FilterResultTest, ErrorContext_WithFilterParam_EmptyStrings)
{
    ErrorContext ctx;
    ctx.withFilterParam("", "");
    
    EXPECT_TRUE(ctx.filter_params.has_value());
    const std::string params = ctx.filter_params.value();
    EXPECT_EQ(params, "=");
}

/**
 * @brief Тест ErrorContext::withFilterParam() с очень длинными значениями
 */
TEST_F(FilterResultTest, ErrorContext_WithFilterParam_LongValues)
{
    ErrorContext ctx;
    const std::string long_value(1000, 'a');
    ctx.withFilterParam("param", long_value);
    
    EXPECT_TRUE(ctx.filter_params.has_value());
    const std::string params = ctx.filter_params.value();
    EXPECT_NE(params.find("param="), std::string::npos);
    EXPECT_EQ(params.length(), 6 + long_value.length()); // "param=" + значение
}

/**
 * @brief Тест ErrorContext::withImage() с граничными значениями
 */
TEST_F(FilterResultTest, ErrorContext_WithImage_BoundaryValues)
{
    // Тест с минимальными значениями
    {
        ErrorContext ctx = ErrorContext::withImage(1, 1, 1);
        EXPECT_EQ(ctx.image_width.value(), 1);
        EXPECT_EQ(ctx.image_height.value(), 1);
        EXPECT_EQ(ctx.image_channels.value(), 1);
    }
    
    // Тест с большими значениями
    {
        ErrorContext ctx = ErrorContext::withImage(10000, 10000, 4);
        EXPECT_EQ(ctx.image_width.value(), 10000);
        EXPECT_EQ(ctx.image_height.value(), 10000);
        EXPECT_EQ(ctx.image_channels.value(), 4);
    }
    
    // Тест с нулевыми значениями (допустимо для контекста)
    {
        ErrorContext ctx = ErrorContext::withImage(0, 0, 0);
        EXPECT_EQ(ctx.image_width.value(), 0);
        EXPECT_EQ(ctx.image_height.value(), 0);
        EXPECT_EQ(ctx.image_channels.value(), 0);
    }
}

/**
 * @brief Тест ErrorContext::withFilename() с различными именами файлов
 */
TEST_F(FilterResultTest, ErrorContext_WithFilename_VariousNames)
{
    // Тест с обычным именем
    {
        ErrorContext ctx = ErrorContext::withFilename("test.png");
        EXPECT_EQ(ctx.filename.value(), "test.png");
    }
    
    // Тест с путем
    {
        ErrorContext ctx = ErrorContext::withFilename("/path/to/test.png");
        EXPECT_EQ(ctx.filename.value(), "/path/to/test.png");
    }
    
    // Тест с пустой строкой
    {
        ErrorContext ctx = ErrorContext::withFilename("");
        EXPECT_EQ(ctx.filename.value(), "");
    }
    
    // Тест с длинным именем
    {
        const std::string long_name(500, 'a');
        ErrorContext ctx = ErrorContext::withFilename(long_name);
        EXPECT_EQ(ctx.filename.value(), long_name);
    }
}

/**
 * @brief Тест ErrorContext конструктора по умолчанию
 */
TEST_F(FilterResultTest, ErrorContext_DefaultConstructor)
{
    ErrorContext ctx;
    EXPECT_FALSE(ctx.filename.has_value());
    EXPECT_FALSE(ctx.image_width.has_value());
    EXPECT_FALSE(ctx.image_height.has_value());
    EXPECT_FALSE(ctx.image_channels.has_value());
    EXPECT_FALSE(ctx.filter_params.has_value());
    EXPECT_FALSE(ctx.system_error_code.has_value());
}

/**
 * @brief Тест ErrorContext с множественными вызовами withFilterParam и различными типами
 */
TEST_F(FilterResultTest, ErrorContext_WithFilterParam_MixedTypes)
{
    ErrorContext ctx;
    ctx.withFilterParam("int_param", 42)
       .withFilterParam("double_param", 3.14)
       .withFilterParam("string_param", "value")
       .withFilterParam("float_param", 2.5f)
       .withFilterParam("bool_param", true);
    
    EXPECT_TRUE(ctx.filter_params.has_value());
    const std::string params = ctx.filter_params.value();
    EXPECT_NE(params.find("int_param="), std::string::npos);
    EXPECT_NE(params.find("double_param="), std::string::npos);
    EXPECT_NE(params.find("string_param=value"), std::string::npos);
    EXPECT_NE(params.find("float_param="), std::string::npos);
    EXPECT_NE(params.find("bool_param="), std::string::npos);
    
    // Проверяем, что все параметры разделены запятыми
    size_t comma_count = 0;
    size_t pos = 0;
    while ((pos = params.find(", ", pos)) != std::string::npos)
    {
        comma_count++;
        pos += 2;
    }
    EXPECT_EQ(comma_count, 4); // 5 параметров = 4 запятые
}

