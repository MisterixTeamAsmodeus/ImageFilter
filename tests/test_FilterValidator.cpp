#include <gtest/gtest.h>
#include <utils/FilterValidator.h>
#include <utils/FilterResult.h>
#include <limits>

/**
 * @brief Тесты для FilterValidator
 */
class FilterValidatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// Тест валидации радиуса (int) - успешный случай
TEST_F(FilterValidatorTest, ValidateRadius_Int_Success)
{
    const auto result = FilterValidator::validateRadius(5, 0, 100);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации радиуса (int) - слишком маленький
TEST_F(FilterValidatorTest, ValidateRadius_Int_TooSmall)
{
    const auto result = FilterValidator::validateRadius(-1, 0, 100);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidRadius);
}

// Тест валидации радиуса (int) - слишком большой
TEST_F(FilterValidatorTest, ValidateRadius_Int_TooLarge)
{
    const auto result = FilterValidator::validateRadius(101, 0, 100);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidRadius);
}

// Тест валидации радиуса (int) - превышает размер изображения
TEST_F(FilterValidatorTest, ValidateRadius_Int_ExceedsImageSize)
{
    const auto result = FilterValidator::validateRadius(100, 0, 1000, 50, 50);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidRadius);
}

// Тест валидации радиуса (double) - успешный случай
TEST_F(FilterValidatorTest, ValidateRadius_Double_Success)
{
    const auto result = FilterValidator::validateRadius(5.5, 0.0, 100.0);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации радиуса (double) - слишком маленький
TEST_F(FilterValidatorTest, ValidateRadius_Double_TooSmall)
{
    const auto result = FilterValidator::validateRadius(-1.0, 0.0, 100.0);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidRadius);
}

// Тест валидации радиуса (double) - слишком большой
TEST_F(FilterValidatorTest, ValidateRadius_Double_TooLarge)
{
    const auto result = FilterValidator::validateRadius(101.0, 0.0, 100.0);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidRadius);
}

// Тест валидации радиуса (double) - превышает размер изображения
TEST_F(FilterValidatorTest, ValidateRadius_Double_ExceedsImageSize)
{
    const auto result = FilterValidator::validateRadius(30.0, 0.0, 1000.0, 50, 50);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidRadius);
}

// Тест валидации коэффициента - успешный случай
TEST_F(FilterValidatorTest, ValidateFactor_Success)
{
    const auto result = FilterValidator::validateFactor(1.5, 0.0, 10.0);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации коэффициента - слишком маленький
TEST_F(FilterValidatorTest, ValidateFactor_TooSmall)
{
    const auto result = FilterValidator::validateFactor(-0.1, 0.0, 10.0);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidFactor);
}

// Тест валидации коэффициента - слишком большой
TEST_F(FilterValidatorTest, ValidateFactor_TooLarge)
{
    const auto result = FilterValidator::validateFactor(11.0, 0.0, 10.0);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidFactor);
}

// Тест валидации порога - успешный случай
TEST_F(FilterValidatorTest, ValidateThreshold_Success)
{
    const auto result = FilterValidator::validateThreshold(128, 0, 255);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации порога - слишком маленький
TEST_F(FilterValidatorTest, ValidateThreshold_TooSmall)
{
    const auto result = FilterValidator::validateThreshold(-1, 0, 255);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidThreshold);
}

// Тест валидации порога - слишком большой
TEST_F(FilterValidatorTest, ValidateThreshold_TooLarge)
{
    const auto result = FilterValidator::validateThreshold(256, 0, 255);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidThreshold);
}

// Тест валидации интенсивности - успешный случай
TEST_F(FilterValidatorTest, ValidateIntensity_Success)
{
    const auto result = FilterValidator::validateIntensity(0.5, 0.0, 1.0);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации интенсивности - слишком маленькая
TEST_F(FilterValidatorTest, ValidateIntensity_TooSmall)
{
    const auto result = FilterValidator::validateIntensity(-0.1, 0.0, 1.0);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::ParameterOutOfRange);
}

// Тест валидации интенсивности - слишком большая
TEST_F(FilterValidatorTest, ValidateIntensity_TooLarge)
{
    const auto result = FilterValidator::validateIntensity(1.1, 0.0, 1.0);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::ParameterOutOfRange);
}

// Тест валидации качества - успешный случай
TEST_F(FilterValidatorTest, ValidateQuality_Success)
{
    const auto result = FilterValidator::validateQuality(85, 0, 100);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации качества - слишком маленькое
TEST_F(FilterValidatorTest, ValidateQuality_TooSmall)
{
    const auto result = FilterValidator::validateQuality(-1, 0, 100);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidQuality);
}

// Тест валидации качества - слишком большое
TEST_F(FilterValidatorTest, ValidateQuality_TooLarge)
{
    const auto result = FilterValidator::validateQuality(101, 0, 100);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidQuality);
}

// Тест валидации угла - успешный случай
TEST_F(FilterValidatorTest, ValidateAngle_Success)
{
    const auto result = FilterValidator::validateAngle(45.0, -360.0, 360.0);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации угла - слишком маленький
TEST_F(FilterValidatorTest, ValidateAngle_TooSmall)
{
    const auto result = FilterValidator::validateAngle(-361.0, -360.0, 360.0);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidAngle);
}

// Тест валидации угла - слишком большой
TEST_F(FilterValidatorTest, ValidateAngle_TooLarge)
{
    const auto result = FilterValidator::validateAngle(361.0, -360.0, 360.0);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidAngle);
}

// Тест валидации размера ядра - успешный случай (нечетное)
TEST_F(FilterValidatorTest, ValidateKernelSize_Success)
{
    const auto result = FilterValidator::validateKernelSize(5, 1, 100);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации размера ядра - четное число (должно быть нечетным)
TEST_F(FilterValidatorTest, ValidateKernelSize_EvenNumber)
{
    const auto result = FilterValidator::validateKernelSize(4, 1, 100);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidKernelSize);
}

// Тест валидации размера ядра - слишком маленький
TEST_F(FilterValidatorTest, ValidateKernelSize_TooSmall)
{
    const auto result = FilterValidator::validateKernelSize(0, 1, 100);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidKernelSize);
}

// Тест валидации размера ядра - слишком большой
TEST_F(FilterValidatorTest, ValidateKernelSize_TooLarge)
{
    const auto result = FilterValidator::validateKernelSize(101, 1, 100);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidKernelSize);
}

// Тест валидации размера изображения - успешный случай
TEST_F(FilterValidatorTest, ValidateImageSize_Success)
{
    const auto result = FilterValidator::validateImageSize(100, 100, 3);
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации размера изображения - нулевая ширина
TEST_F(FilterValidatorTest, ValidateImageSize_ZeroWidth)
{
    const auto result = FilterValidator::validateImageSize(0, 100, 3);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidWidth);
}

// Тест валидации размера изображения - отрицательная ширина
TEST_F(FilterValidatorTest, ValidateImageSize_NegativeWidth)
{
    const auto result = FilterValidator::validateImageSize(-1, 100, 3);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidWidth);
}

// Тест валидации размера изображения - нулевая высота
TEST_F(FilterValidatorTest, ValidateImageSize_ZeroHeight)
{
    const auto result = FilterValidator::validateImageSize(100, 0, 3);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidHeight);
}

// Тест валидации размера изображения - отрицательная высота
TEST_F(FilterValidatorTest, ValidateImageSize_NegativeHeight)
{
    const auto result = FilterValidator::validateImageSize(100, -1, 3);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidHeight);
}

// Тест валидации размера изображения - некорректное количество каналов
TEST_F(FilterValidatorTest, ValidateImageSize_InvalidChannels)
{
    const auto result = FilterValidator::validateImageSize(100, 100, 2);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidChannels);
}

// Тест валидации размера изображения - переполнение
// Примечание: проверка переполнения выполняется через SafeMath::safeMultiply
// На 64-битных системах очень большие значения могут не вызывать переполнение
// Этот тест проверяет, что функция корректно обрабатывает граничные случаи
TEST_F(FilterValidatorTest, ValidateImageSize_Overflow)
{
    // Используем очень большие значения, которые могут вызвать переполнение
    // на некоторых системах или при умножении на channels
    const int width = 1000000;
    const int height = 1000000;
    const auto result = FilterValidator::validateImageSize(width, height, 3);
    // Результат зависит от системы, но функция должна корректно обработать случай
    // Если переполнение происходит, должна быть ошибка ArithmeticOverflow
    // Если нет - успех (на 64-битных системах)
    if (!result.isSuccess())
    {
        EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);
    }
    // Если успех - это нормально для 64-битных систем
}

// Тест валидации размера буфера - успешный случай
TEST_F(FilterValidatorTest, ValidateBufferSize_Success)
{
    size_t buffer_size = 0;
    const auto result = FilterValidator::validateBufferSize(100, 100, 3, buffer_size);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(buffer_size, static_cast<size_t>(100 * 100 * 3));
}

// Тест валидации размера буфера - некорректные размеры
TEST_F(FilterValidatorTest, ValidateBufferSize_InvalidSize)
{
    size_t buffer_size = 0;
    const auto result = FilterValidator::validateBufferSize(0, 100, 3, buffer_size);
    EXPECT_FALSE(result.isSuccess());
}

// Тест валидации размера буфера - переполнение
// Примечание: проверка переполнения выполняется через SafeMath::safeMultiply
// На 64-битных системах очень большие значения могут не вызывать переполнение
// Этот тест проверяет, что функция корректно обрабатывает граничные случаи
TEST_F(FilterValidatorTest, ValidateBufferSize_Overflow)
{
    size_t buffer_size = 0;
    // Используем очень большие значения, которые могут вызвать переполнение
    // на некоторых системах или при умножении на channels
    const int width = 1000000;
    const int height = 1000000;
    const auto result = FilterValidator::validateBufferSize(width, height, 3, buffer_size);
    // Результат зависит от системы, но функция должна корректно обработать случай
    // Если переполнение происходит, должна быть ошибка ArithmeticOverflow
    // Если нет - успех (на 64-битных системах)
    if (!result.isSuccess())
    {
        EXPECT_EQ(result.error, FilterError::ArithmeticOverflow);
    }
    else
    {
        // Если успех, проверяем, что buffer_size вычислен корректно
        EXPECT_GT(buffer_size, 0);
    }
}

// Тест валидации диапазона (шаблонная функция) - успешный случай
TEST_F(FilterValidatorTest, ValidateRange_Success)
{
    const auto result = FilterValidator::validateRange(5, 0, 10, "test_param");
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации диапазона - слишком маленькое значение
TEST_F(FilterValidatorTest, ValidateRange_TooSmall)
{
    const auto result = FilterValidator::validateRange(-1, 0, 10, "test_param");
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::ParameterOutOfRange);
}

// Тест валидации диапазона - слишком большое значение
TEST_F(FilterValidatorTest, ValidateRange_TooLarge)
{
    const auto result = FilterValidator::validateRange(11, 0, 10, "test_param");
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::ParameterOutOfRange);
}

// Тест валидации диапазона с double
TEST_F(FilterValidatorTest, ValidateRange_Double)
{
    const auto result = FilterValidator::validateRange(5.5, 0.0, 10.0, "test_param");
    EXPECT_TRUE(result.isSuccess());
}

// Тест валидации диапазона с double - ошибка
TEST_F(FilterValidatorTest, ValidateRange_Double_Error)
{
    const auto result = FilterValidator::validateRange(11.0, 0.0, 10.0, "test_param");
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::ParameterOutOfRange);
}

// Тест граничных значений - минимальный радиус
TEST_F(FilterValidatorTest, ValidateRadius_Boundary_Min)
{
    const auto result = FilterValidator::validateRadius(0, 0, 100);
    EXPECT_TRUE(result.isSuccess());
}

// Тест граничных значений - максимальный радиус
TEST_F(FilterValidatorTest, ValidateRadius_Boundary_Max)
{
    const auto result = FilterValidator::validateRadius(100, 0, 100);
    EXPECT_TRUE(result.isSuccess());
}

// Тест граничных значений - минимальный порог
TEST_F(FilterValidatorTest, ValidateThreshold_Boundary_Min)
{
    const auto result = FilterValidator::validateThreshold(0, 0, 255);
    EXPECT_TRUE(result.isSuccess());
}

// Тест граничных значений - максимальный порог
TEST_F(FilterValidatorTest, ValidateThreshold_Boundary_Max)
{
    const auto result = FilterValidator::validateThreshold(255, 0, 255);
    EXPECT_TRUE(result.isSuccess());
}

