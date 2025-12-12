#include <gtest/gtest.h>
#include <utils/LookupTables.h>
#include <cmath>
#include <numbers>

/**
 * @brief Тесты для LookupTables
 */
class LookupTablesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Инициализируем таблицы перед тестами
        LookupTables::initialize();
    }
};

// Тест инициализации таблиц
TEST_F(LookupTablesTest, Initialize)
{
    // Инициализация должна быть идемпотентной
    LookupTables::initialize();
    LookupTables::initialize();
    
    // После инициализации таблицы должны работать
    const auto sin_val = LookupTables::sin(0);
    EXPECT_DOUBLE_EQ(sin_val, 0.0);
}

// Тест sin (double)
TEST_F(LookupTablesTest, SinDouble)
{
    EXPECT_NEAR(LookupTables::sin(0), 0.0, 1e-6);
    EXPECT_NEAR(LookupTables::sin(90), 1.0, 1e-6);
    EXPECT_NEAR(LookupTables::sin(180), 0.0, 1e-6);
    EXPECT_NEAR(LookupTables::sin(270), -1.0, 1e-6);
    EXPECT_NEAR(LookupTables::sin(360), 0.0, 1e-6);
    
    // Тест нормализации углов
    EXPECT_NEAR(LookupTables::sin(450), 1.0, 1e-6);  // 450° = 90°
    EXPECT_NEAR(LookupTables::sin(-90), -1.0, 1e-6);  // -90° = 270°
}

// Тест cos (double)
TEST_F(LookupTablesTest, CosDouble)
{
    EXPECT_NEAR(LookupTables::cos(0), 1.0, 1e-6);
    EXPECT_NEAR(LookupTables::cos(90), 0.0, 1e-6);
    EXPECT_NEAR(LookupTables::cos(180), -1.0, 1e-6);
    EXPECT_NEAR(LookupTables::cos(270), 0.0, 1e-6);
    EXPECT_NEAR(LookupTables::cos(360), 1.0, 1e-6);
    
    // Тест нормализации углов
    EXPECT_NEAR(LookupTables::cos(450), 0.0, 1e-6);  // 450° = 90°
    EXPECT_NEAR(LookupTables::cos(-90), 0.0, 1e-6);  // -90° = 270°
}

// Тест sinScaled (масштабированный)
TEST_F(LookupTablesTest, SinScaled)
{
    const auto sin_0 = LookupTables::sinScaled(0);
    EXPECT_EQ(sin_0, 0);
    
    const auto sin_90 = LookupTables::sinScaled(90);
    EXPECT_NEAR(static_cast<double>(sin_90) / 65536.0, 1.0, 1e-4);
    
    const auto sin_180 = LookupTables::sinScaled(180);
    EXPECT_EQ(sin_180, 0);
    
    const auto sin_270 = LookupTables::sinScaled(270);
    EXPECT_NEAR(static_cast<double>(sin_270) / 65536.0, -1.0, 1e-4);
}

// Тест cosScaled (масштабированный)
TEST_F(LookupTablesTest, CosScaled)
{
    const auto cos_0 = LookupTables::cosScaled(0);
    EXPECT_NEAR(static_cast<double>(cos_0) / 65536.0, 1.0, 1e-4);
    
    const auto cos_90 = LookupTables::cosScaled(90);
    EXPECT_EQ(cos_90, 0);
    
    const auto cos_180 = LookupTables::cosScaled(180);
    EXPECT_NEAR(static_cast<double>(cos_180) / 65536.0, -1.0, 1e-4);
}

// Тест expNegative
TEST_F(LookupTablesTest, ExpNegative)
{
    EXPECT_NEAR(LookupTables::expNegative(0.0), 1.0, 1e-6);
    EXPECT_NEAR(LookupTables::expNegative(1.0), std::exp(-1.0), 1e-4);
    EXPECT_NEAR(LookupTables::expNegative(2.0), std::exp(-2.0), 1e-4);
    
    // Тест отрицательных значений (должно возвращать exp(x))
    EXPECT_NEAR(LookupTables::expNegative(-1.0), std::exp(1.0), 1e-4);
    
    // Тест больших значений (должно возвращать 0)
    EXPECT_NEAR(LookupTables::expNegative(25.0), 0.0, 1e-6);
}

// Тест sqrtInt
TEST_F(LookupTablesTest, SqrtInt)
{
    EXPECT_NEAR(LookupTables::sqrtInt(0), 0.0, 1e-6);
    EXPECT_NEAR(LookupTables::sqrtInt(1), 1.0, 1e-6);
    EXPECT_NEAR(LookupTables::sqrtInt(4), 2.0, 1e-6);
    EXPECT_NEAR(LookupTables::sqrtInt(9), 3.0, 1e-6);
    EXPECT_NEAR(LookupTables::sqrtInt(100), 10.0, 1e-6);
    
    // Тест отрицательных значений
    EXPECT_EQ(LookupTables::sqrtInt(-1), 0.0);
    
    // Тест значений вне таблицы (должно использовать std::sqrt)
    EXPECT_NEAR(LookupTables::sqrtInt(20000), std::sqrt(20000.0), 1e-4);
}

// Тест sqrtIntScaled
TEST_F(LookupTablesTest, SqrtIntScaled)
{
    const auto sqrt_0 = LookupTables::sqrtIntScaled(0);
    EXPECT_EQ(sqrt_0, 0);
    
    const auto sqrt_1 = LookupTables::sqrtIntScaled(1);
    EXPECT_NEAR(static_cast<double>(sqrt_1) / 65536.0, 1.0, 1e-4);
    
    const auto sqrt_4 = LookupTables::sqrtIntScaled(4);
    EXPECT_NEAR(static_cast<double>(sqrt_4) / 65536.0, 2.0, 1e-4);
    
    // Тест отрицательных значений
    EXPECT_EQ(LookupTables::sqrtIntScaled(-1), 0);
    
    // Тест значений вне таблицы
    const auto sqrt_20000 = LookupTables::sqrtIntScaled(20000);
    EXPECT_NEAR(static_cast<double>(sqrt_20000) / 65536.0, std::sqrt(20000.0), 1e-4);
}

// Тест getGammaLUT
TEST_F(LookupTablesTest, GetGammaLUT)
{
    auto lut = LookupTables::getGammaLUT(1.0);
    EXPECT_EQ(lut.size(), 256);
    
    // При gamma=1.0 значения не должны изменяться
    for (int i = 0; i < 256; ++i)
    {
        EXPECT_EQ(lut[static_cast<size_t>(i)], static_cast<uint8_t>(i));
    }
    
    // Тест с другим значением gamma
    auto lut_gamma2 = LookupTables::getGammaLUT(2.0);
    EXPECT_EQ(lut_gamma2.size(), 256);
    
    // При gamma=2.0 значения должны изменяться (не все равны индексу)
    // Проверяем, что таблица не идентична исходной
    bool all_equal = true;
    for (int i = 0; i < 256; ++i)
    {
        if (lut_gamma2[static_cast<size_t>(i)] != static_cast<uint8_t>(i))
        {
            all_equal = false;
            break;
        }
    }
    EXPECT_FALSE(all_equal);
    
    // Тест некорректных значений (должны нормализоваться к 1.0)
    auto lut_invalid = LookupTables::getGammaLUT(-1.0);
    EXPECT_EQ(lut_invalid.size(), 256);
    // Должна быть такая же, как при gamma=1.0
    for (int i = 0; i < 256; ++i)
    {
        EXPECT_EQ(lut_invalid[static_cast<size_t>(i)], static_cast<uint8_t>(i));
    }
}

// Тест getBrightnessLUT
TEST_F(LookupTablesTest, GetBrightnessLUT)
{
    auto lut = LookupTables::getBrightnessLUT(0.0);
    EXPECT_EQ(lut.size(), 256);
    
    // При brightness=0.0 значения не должны изменяться
    for (int i = 0; i < 256; ++i)
    {
        EXPECT_EQ(lut[static_cast<size_t>(i)], static_cast<uint8_t>(i));
    }
    
    // Тест положительной яркости
    auto lut_bright = LookupTables::getBrightnessLUT(0.5);
    EXPECT_EQ(lut_bright.size(), 256);
    EXPECT_GT(lut_bright[128], 128);
    
    // Тест отрицательной яркости
    auto lut_dark = LookupTables::getBrightnessLUT(-0.5);
    EXPECT_EQ(lut_dark.size(), 256);
    EXPECT_LT(lut_dark[128], 128);
    
    // Тест граничных значений (должны ограничиваться)
    auto lut_max = LookupTables::getBrightnessLUT(2.0);
    EXPECT_EQ(lut_max.size(), 256);
    EXPECT_EQ(lut_max[255], 255);
    
    auto lut_min = LookupTables::getBrightnessLUT(-2.0);
    EXPECT_EQ(lut_min.size(), 256);
    EXPECT_EQ(lut_min[0], 0);
}

// Тест getContrastLUT
TEST_F(LookupTablesTest, GetContrastLUT)
{
    auto lut = LookupTables::getContrastLUT(0.0);
    EXPECT_EQ(lut.size(), 256);
    
    // При contrast=0.0 значения не должны изменяться
    for (int i = 0; i < 256; ++i)
    {
        EXPECT_EQ(lut[static_cast<size_t>(i)], static_cast<uint8_t>(i));
    }
    
    // Тест положительного контраста
    auto lut_high = LookupTables::getContrastLUT(0.5);
    EXPECT_EQ(lut_high.size(), 256);
    // Высокий контраст увеличивает разницу между светлыми и темными
    EXPECT_GT(lut_high[200], 200);
    EXPECT_LT(lut_high[50], 50);
    
    // Тест отрицательного контраста
    auto lut_low = LookupTables::getContrastLUT(-0.5);
    EXPECT_EQ(lut_low.size(), 256);
    // Низкий контраст уменьшает разницу
    EXPECT_LT(lut_low[200], 200);
    EXPECT_GT(lut_low[50], 50);
    
    // Тест граничных значений
    auto lut_max = LookupTables::getContrastLUT(2.0);
    EXPECT_EQ(lut_max.size(), 256);
    
    auto lut_min = LookupTables::getContrastLUT(-2.0);
    EXPECT_EQ(lut_min.size(), 256);
}

// Тест точности таблиц
TEST_F(LookupTablesTest, Accuracy)
{
    // Проверяем точность sin/cos для различных углов
    for (int angle = 0; angle < 360; angle += 15)
    {
        const double expected_sin = std::sin(angle * std::numbers::pi / 180.0);
        const double expected_cos = std::cos(angle * std::numbers::pi / 180.0);
        
        EXPECT_NEAR(LookupTables::sin(angle), expected_sin, 1e-5);
        EXPECT_NEAR(LookupTables::cos(angle), expected_cos, 1e-5);
    }
}

