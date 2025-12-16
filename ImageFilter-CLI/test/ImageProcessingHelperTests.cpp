/**
 * @file ImageProcessingHelperTests.cpp
 * @brief Юнит- и лёгкие интеграционные тесты для вспомогательного класса CLI ImageProcessingHelper.
 *
 * Тесты охватывают:
 * - разбор строки с цепочкой фильтров;
 * - обработку некорректного качества JPEG без обращения к файловой системе.
 */

#include <gtest/gtest.h>

#include <cli/ImageProcessingHelper.h>

/**
 * @brief Проверка корректного разбора цепочки фильтров с пробелами и пустыми элементами.
 */
TEST(ImageProcessingHelperTests, ParseFilterChain_Basic)
{
    const std::string filter_chain = "grayscale,  brightness , ,contrast";

    const std::vector<std::string> filters = ImageProcessingHelper::parseFilterChain(filter_chain);

    ASSERT_EQ(filters.size(), 3U);
    EXPECT_EQ(filters[0], "grayscale");
    EXPECT_EQ(filters[1], "brightness");
    EXPECT_EQ(filters[2], "contrast");
}

/**
 * @brief Проверка обработки некорректного качества JPEG без взаимодействия с файловой системой.
 *
 * Ожидается, что метод вернёт false уже на этапе установки качества JPEG,
 * не пытаясь загрузить/сохранить изображение.
 */
TEST(ImageProcessingHelperTests, ProcessSingleImage_InvalidJpegQuality)
{
    CLI::App app("test-app");

    const std::vector<std::string> filters;

    const bool result = ImageProcessingHelper::processSingleImage(
        "input.jpg",
        "output.jpg",
        filters,
        app,
        true,
        false,
        -1);

    EXPECT_FALSE(result);
}


