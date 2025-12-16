/**
 * @file SafeMathTests.cpp
 * @brief Набор юнит-тестов для утилит безопасной математики SafeMath.
 *
 * Тесты покрывают корректные сценарии и случаи переполнения
 * для операций умножения, деления, сложения и вычитания.
 */

#include <gtest/gtest.h>

#include <utils/SafeMath.h>

/**
 * @brief Проверка безопасного умножения без переполнения.
 */
TEST(SafeMathTests, SafeMultiply_NoOverflow)
{
    int result = 0;
    const bool ok = SafeMath::safeMultiply(1000, 2000, result);

    ASSERT_TRUE(ok);
    EXPECT_EQ(result, 2'000'000);
}

/**
 * @brief Проверка безопасного умножения с переполнением.
 */
TEST(SafeMathTests, SafeMultiply_Overflow)
{
    unsigned int result = 0;
    const bool ok = SafeMath::safeMultiply(
        std::numeric_limits<unsigned int>::max(),
        2U,
        result);

    EXPECT_FALSE(ok);
}

/**
 * @brief Проверка безопасного деления без деления на ноль.
 */
TEST(SafeMathTests, SafeDivide_NoZero)
{
    int result = 0;
    const bool ok = SafeMath::safeDivide(10, 2, result);

    ASSERT_TRUE(ok);
    EXPECT_EQ(result, 5);
}

/**
 * @brief Проверка безопасного деления на ноль.
 */
TEST(SafeMathTests, SafeDivide_ByZero)
{
    int result = 0;
    const bool ok = SafeMath::safeDivide(10, 0, result);

    EXPECT_FALSE(ok);
}

/**
 * @brief Проверка безопасного сложения без переполнения.
 */
TEST(SafeMathTests, SafeAdd_NoOverflow)
{
    int result = 0;
    const bool ok = SafeMath::safeAdd(10, 20, result);

    ASSERT_TRUE(ok);
    EXPECT_EQ(result, 30);
}

/**
 * @brief Проверка безопасного сложения с переполнением.
 */
TEST(SafeMathTests, SafeAdd_Overflow)
{
    int result = 0;
    const bool ok = SafeMath::safeAdd(
        std::numeric_limits<int>::max(),
        1,
        result);

    EXPECT_FALSE(ok);
}

/**
 * @brief Проверка безопасного вычитания без переполнения.
 */
TEST(SafeMathTests, SafeSubtract_NoOverflow)
{
    int result = 0;
    const bool ok = SafeMath::safeSubtract(20, 10, result);

    ASSERT_TRUE(ok);
    EXPECT_EQ(result, 10);
}

/**
 * @brief Проверка безопасного вычитания с переполнением.
 */
TEST(SafeMathTests, SafeSubtract_Overflow)
{
    int result = 0;
    const bool ok = SafeMath::safeSubtract(
        std::numeric_limits<int>::min(),
        1,
        result);

    EXPECT_FALSE(ok);
}

/**
 * @brief Проверка функции clamp для различных положений значения в диапазоне.
 */
TEST(SafeMathTests, Clamp_BasicCases)
{
    EXPECT_EQ(SafeMath::clamp(5, 0, 10), 5);
    EXPECT_EQ(SafeMath::clamp(-1, 0, 10), 0);
    EXPECT_EQ(SafeMath::clamp(20, 0, 10), 10);
}


