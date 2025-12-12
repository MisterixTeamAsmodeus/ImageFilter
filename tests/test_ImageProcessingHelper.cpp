#include <gtest/gtest.h>
#include <cli/ImageProcessingHelper.h>
#include <ImageProcessor.h>
#include <utils/FilterFactory.h>
#include <CLI/CLI.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

/**
 * @brief Тесты для ImageProcessingHelper
 */
class ImageProcessingHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_dir_ = fs::temp_directory_path() / "ImageFilterProcessingTests";
        input_dir_ = test_dir_ / "input";
        output_dir_ = test_dir_ / "output";
        
        fs::create_directories(input_dir_);
        fs::create_directories(output_dir_);
        
        // Регистрируем фильтры
        auto& factory = FilterFactory::getInstance();
        factory.registerAll();
        
        // Создаем тестовое изображение
        createTestImage();
    }
    
    void TearDown() override
    {
        if (fs::exists(test_dir_))
        {
            fs::remove_all(test_dir_);
        }
    }
    
    void createTestImage()
    {
        // Создаем простое тестовое изображение через ImageProcessor
        ImageProcessor processor;
        
        constexpr int width = 10;
        constexpr int height = 10;
        constexpr int channels = 3;
        
        auto* data = static_cast<uint8_t*>(std::malloc(width * height * channels));
        ASSERT_NE(data, nullptr);
        
        // Заполняем данными
        for (int i = 0; i < width * height * channels; ++i)
        {
            data[i] = static_cast<uint8_t>(i % 256);
        }
        
        processor.resize(width, height, channels, data);
        
        input_file_ = input_dir_ / "test.png";
        processor.saveToFile(input_file_.string());
        
        std::free(data);
    }
    
    fs::path test_dir_;
    fs::path input_dir_;
    fs::path output_dir_;
    fs::path input_file_;
};

// Тест parseFilterChain
TEST_F(ImageProcessingHelperTest, ParseFilterChain)
{
    std::string chain = "grayscale,sepia,invert";
    auto filters = ImageProcessingHelper::parseFilterChain(chain);
    
    EXPECT_EQ(filters.size(), 3);
    EXPECT_EQ(filters[0], "grayscale");
    EXPECT_EQ(filters[1], "sepia");
    EXPECT_EQ(filters[2], "invert");
}

// Тест parseFilterChain с пробелами
TEST_F(ImageProcessingHelperTest, ParseFilterChainWithSpaces)
{
    std::string chain = "grayscale , sepia , invert";
    auto filters = ImageProcessingHelper::parseFilterChain(chain);
    
    EXPECT_EQ(filters.size(), 3);
    EXPECT_EQ(filters[0], "grayscale");
    EXPECT_EQ(filters[1], "sepia");
    EXPECT_EQ(filters[2], "invert");
}

// Тест parseFilterChain с пустой строкой
TEST_F(ImageProcessingHelperTest, ParseFilterChainEmpty)
{
    std::string chain = "";
    auto filters = ImageProcessingHelper::parseFilterChain(chain);
    
    EXPECT_TRUE(filters.empty());
}

// Тест parseFilterChain с одним фильтром
TEST_F(ImageProcessingHelperTest, ParseFilterChainSingle)
{
    std::string chain = "grayscale";
    auto filters = ImageProcessingHelper::parseFilterChain(chain);
    
    EXPECT_EQ(filters.size(), 1);
    EXPECT_EQ(filters[0], "grayscale");
}

// Тест parseFilterChain с пустыми элементами
TEST_F(ImageProcessingHelperTest, ParseFilterChainEmptyElements)
{
    std::string chain = "grayscale,,sepia";
    auto filters = ImageProcessingHelper::parseFilterChain(chain);
    
    // Пустые элементы должны быть проигнорированы
    EXPECT_EQ(filters.size(), 2);
    EXPECT_EQ(filters[0], "grayscale");
    EXPECT_EQ(filters[1], "sepia");
}

// Тест processSingleImage с одним фильтром
TEST_F(ImageProcessingHelperTest, ProcessSingleImageSingleFilter)
{
    CLI::App app("test");
    auto output_file = output_dir_ / "output.png";
    
    std::vector<std::string> filters = {"grayscale"};
    
    bool result = ImageProcessingHelper::processSingleImage(
        input_file_.string(),
        output_file.string(),
        filters,
        app,
        false,  // preserve_alpha
        false,  // force_rgb
        90      // jpeg_quality
    );
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(fs::exists(output_file));
}

// Тест processSingleImage с несколькими фильтрами
TEST_F(ImageProcessingHelperTest, ProcessSingleImageMultipleFilters)
{
    CLI::App app("test");
    auto output_file = output_dir_ / "output.png";
    
    std::vector<std::string> filters = {"grayscale", "sepia"};
    
    bool result = ImageProcessingHelper::processSingleImage(
        input_file_.string(),
        output_file.string(),
        filters,
        app,
        false,
        false,
        90
    );
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(fs::exists(output_file));
}

// Тест processSingleImage с несуществующим входным файлом
TEST_F(ImageProcessingHelperTest, ProcessSingleImageNonExistentInput)
{
    CLI::App app("test");
    auto output_file = output_dir_ / "output.png";
    
    std::vector<std::string> filters = {"grayscale"};
    
    bool result = ImageProcessingHelper::processSingleImage(
        "/nonexistent/file.png",
        output_file.string(),
        filters,
        app,
        false,
        false,
        90
    );
    
    EXPECT_FALSE(result);
}

// Тест processSingleImage с невалидным фильтром
TEST_F(ImageProcessingHelperTest, ProcessSingleImageInvalidFilter)
{
    CLI::App app("test");
    auto output_file = output_dir_ / "output.png";
    
    std::vector<std::string> filters = {"nonexistent_filter"};
    
    bool result = ImageProcessingHelper::processSingleImage(
        input_file_.string(),
        output_file.string(),
        filters,
        app,
        false,
        false,
        90
    );
    
    EXPECT_FALSE(result);
}

// Тест processSingleImage с preserve_alpha
TEST_F(ImageProcessingHelperTest, ProcessSingleImagePreserveAlpha)
{
    CLI::App app("test");
    auto output_file = output_dir_ / "output.png";
    
    std::vector<std::string> filters = {"grayscale"};
    
    bool result = ImageProcessingHelper::processSingleImage(
        input_file_.string(),
        output_file.string(),
        filters,
        app,
        true,   // preserve_alpha
        false,
        90
    );
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(fs::exists(output_file));
}

// Тест processSingleImage с force_rgb
TEST_F(ImageProcessingHelperTest, ProcessSingleImageForceRGB)
{
    CLI::App app("test");
    auto output_file = output_dir_ / "output.png";
    
    std::vector<std::string> filters = {"grayscale"};
    
    bool result = ImageProcessingHelper::processSingleImage(
        input_file_.string(),
        output_file.string(),
        filters,
        app,
        false,
        true,   // force_rgb
        90
    );
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(fs::exists(output_file));
}

// Тест processSingleImage с различными значениями jpeg_quality
TEST_F(ImageProcessingHelperTest, ProcessSingleImageJPEGQuality)
{
    CLI::App app("test");
    
    std::vector<int> qualities = {50, 75, 90, 100};
    
    for (int quality : qualities)
    {
        auto output_file = output_dir_ / ("output_" + std::to_string(quality) + ".jpg");
        
        std::vector<std::string> filters = {"grayscale"};
        
        bool result = ImageProcessingHelper::processSingleImage(
            input_file_.string(),
            output_file.string(),
            filters,
            app,
            false,
            false,
            quality
        );
        
        EXPECT_TRUE(result);
        EXPECT_TRUE(fs::exists(output_file));
    }
}

// Тест processSingleImage с пустым списком фильтров
TEST_F(ImageProcessingHelperTest, ProcessSingleImageEmptyFilters)
{
    CLI::App app("test");
    auto output_file = output_dir_ / "output.png";
    
    std::vector<std::string> filters;
    
    bool result = ImageProcessingHelper::processSingleImage(
        input_file_.string(),
        output_file.string(),
        filters,
        app,
        false,
        false,
        90
    );
    
    // Должно работать даже без фильтров (просто копирование)
    EXPECT_TRUE(result);
    EXPECT_TRUE(fs::exists(output_file));
}

// Тест processSingleImage с невалидным jpeg_quality
TEST_F(ImageProcessingHelperTest, ProcessSingleImageInvalidJPEGQuality)
{
    CLI::App app("test");
    auto output_file = output_dir_ / "output.jpg";
    
    std::vector<std::string> filters = {"grayscale"};
    
    // Недопустимое качество (должно быть 0-100)
    bool result = ImageProcessingHelper::processSingleImage(
        input_file_.string(),
        output_file.string(),
        filters,
        app,
        false,
        false,
        150  // Недопустимое значение
    );
    
    EXPECT_FALSE(result);
}

