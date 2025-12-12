#include <gtest/gtest.h>
#include <ImageProcessor.h>
#include <utils/FilterFactory.h>
#include <utils/BatchProcessor.h>
#include <utils/Config.h>
#include <utils/FilterResult.h>
#include <CLI/CLI.hpp>
#include "test_utils.h"
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>

namespace fs = std::filesystem;

/**
 * @brief Базовый класс для интеграционных тестов
 */
class IntegrationTestBase : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Регистрируем все фильтры
        FilterFactory::getInstance().registerAll();
        
        // Создаем временную директорию для тестов
        test_dir_ = fs::temp_directory_path() / "ImageFilterIntegrationTests";
        fs::create_directories(test_dir_);
    }

    void TearDown() override
    {
        // Удаляем временную директорию
        if (fs::exists(test_dir_))
        {
            fs::remove_all(test_dir_);
        }
    }

    fs::path test_dir_;
};

/**
 * @brief Тест полного цикла обработки изображения
 */
TEST_F(IntegrationTestBase, FullImageProcessingCycle)
{
    // Создаем тестовое изображение
    constexpr int width = 100;
    constexpr int height = 100;
    auto test_image = TestUtils::createTestImage(width, height);
    
    // Сохраняем изображение
    std::string input_file = (test_dir_ / "input.jpg").string();
    std::string output_file = (test_dir_ / "output.jpg").string();
    
    ImageProcessor processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    ASSERT_TRUE(processor.resize(width, height, allocated_data).isSuccess());
    ASSERT_TRUE(processor.saveToFile(input_file).isSuccess());
    
    // Загружаем изображение
    ImageProcessor loaded_processor;
    ASSERT_TRUE(loaded_processor.loadFromFile(input_file).isSuccess());
    EXPECT_EQ(loaded_processor.getWidth(), width);
    EXPECT_EQ(loaded_processor.getHeight(), height);
    
    // Применяем фильтр
    auto& factory = FilterFactory::getInstance();
    CLI::App app;
    auto filter = factory.create("grayscale", app);
    ASSERT_NE(filter, nullptr);
    
    const auto result = filter->apply(loaded_processor);
    EXPECT_TRUE(result.isSuccess());
    
    // Сохраняем результат
    ASSERT_TRUE(loaded_processor.saveToFile(output_file).isSuccess());
    
    // Проверяем, что файл создан
    EXPECT_TRUE(fs::exists(output_file));
}

/**
 * @brief Тест цепочки фильтров
 */
TEST_F(IntegrationTestBase, FilterChain)
{
    constexpr int width = 50;
    constexpr int height = 50;
    auto test_image = TestUtils::createTestImage(width, height);
    
    ImageProcessor processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, test_image.data(), test_image.size());
    
    ASSERT_TRUE(processor.resize(width, height, allocated_data).isSuccess());
    
    // Применяем цепочку фильтров
    auto& factory = FilterFactory::getInstance();
    CLI::App app;
    
    // Добавляем опции ДО создания фильтров
    double brightness_factor = 1.5;
    app.add_option("--brightness-factor", brightness_factor);
    
    std::vector<std::string> filter_names = {"grayscale", "invert", "brightness"};
    
    for (const auto& filter_name : filter_names)
    {
        auto filter = factory.create(filter_name, app);
        ASSERT_NE(filter, nullptr);
        
        const auto result = filter->apply(processor);
        EXPECT_TRUE(result.isSuccess()) << "Filter failed: " << filter_name;
    }
    
    EXPECT_TRUE(processor.isValid());
}

/**
 * @brief Тест пакетной обработки
 */
TEST_F(IntegrationTestBase, BatchProcessing)
{
    // Создаем тестовые изображения
    constexpr int width = 50;
    constexpr int height = 50;
    
    fs::path input_dir = test_dir_ / "input";
    fs::path output_dir = test_dir_ / "output";
    fs::create_directories(input_dir);
    fs::create_directories(output_dir);
    
    // Создаем несколько тестовых изображений
    for (int i = 0; i < 3; ++i)
    {
        auto test_image = TestUtils::createTestImage(width, height);
        ImageProcessor processor;
        auto* allocated_data = static_cast<uint8_t*>(std::malloc(test_image.size()));
        ASSERT_NE(allocated_data, nullptr);
        std::memcpy(allocated_data, test_image.data(), test_image.size());
        
        ASSERT_TRUE(processor.resize(width, height, allocated_data).isSuccess());
        
        std::string filename = "test_" + std::to_string(i) + ".jpg";
        std::string filepath = (input_dir / filename).string();
        ASSERT_TRUE(processor.saveToFile(filepath).isSuccess());
    }
    
    // Обрабатываем все файлы
    BatchProcessor batch_processor(input_dir.string(), output_dir.string(), false, "*.jpg");
    
    auto& factory = FilterFactory::getInstance();
    CLI::App app;
    
    auto process_function = [&factory, &app](const std::string& input_path, const std::string& output_path) -> FilterResult {
        ImageProcessor image;
        const auto load_result = image.loadFromFile(input_path);
        if (!load_result.isSuccess())
        {
            return load_result;
        }
        
        auto filter = factory.create("grayscale", app);
        if (!filter)
        {
            return FilterResult::failure(FilterError::InvalidParameter, "Не удалось создать фильтр");
        }
        
        const auto result = filter->apply(image);
        if (!result.isSuccess())
        {
            return result;
        }
        
        return image.saveToFile(output_path);
    };
    
    auto stats = batch_processor.processAll(process_function, nullptr);
    
    EXPECT_EQ(stats.total_files, 3);
    EXPECT_EQ(stats.processed_files, 3);
    EXPECT_EQ(stats.failed_files, 0);
    
    // Проверяем, что выходные файлы созданы
    for (int i = 0; i < 3; ++i)
    {
        std::string filename = "test_" + std::to_string(i) + ".jpg";
        std::string filepath = (output_dir / filename).string();
        EXPECT_TRUE(fs::exists(filepath));
    }
}

/**
 * @brief Тест работы с конфигурацией пресетов
 */
TEST_F(IntegrationTestBase, PresetConfiguration)
{
    Config config;
    
    // Создаем тестовый пресет
    FilterChainConfig preset;
    preset.name = "test_preset";
    preset.description = "Тестовый пресет";
    preset.filters.push_back({"grayscale", {}});
    preset.filters.push_back({"invert", {}});
    
    // Сохраняем пресет
    std::string preset_path = (test_dir_ / "test_preset.json").string();
    ASSERT_TRUE(config.savePreset(preset_path, preset));
    
    // Проверяем, что файл создан
    EXPECT_TRUE(fs::exists(preset_path));
    
    // Загружаем пресет
    auto loaded_preset = config.loadPreset(preset_path);
    ASSERT_NE(loaded_preset, nullptr);
    EXPECT_EQ(loaded_preset->name, preset.name);
    EXPECT_EQ(loaded_preset->description, preset.description);
    EXPECT_EQ(loaded_preset->filters.size(), preset.filters.size());
    
    // Валидируем конфигурацию
    EXPECT_TRUE(config.validateConfig(*loaded_preset));
}

/**
 * @brief Тест обработки с реальными изображениями (симуляция)
 */
TEST_F(IntegrationTestBase, RealImageProcessing)
{
    // Создаем более реалистичное тестовое изображение
    constexpr int width = 200;
    constexpr int height = 200;
    
    // Создаем изображение с градиентом
    std::vector<uint8_t> image_data;
    image_data.reserve(static_cast<size_t>(width) * height * 3);
    
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            image_data.push_back(static_cast<uint8_t>((x * 255) / width)); // R
            image_data.push_back(static_cast<uint8_t>((y * 255) / height)); // G
            image_data.push_back(static_cast<uint8_t>(((x + y) * 255) / (width + height))); // B
        }
    }
    
    ImageProcessor processor;
    auto* allocated_data = static_cast<uint8_t*>(std::malloc(image_data.size()));
    ASSERT_NE(allocated_data, nullptr);
    std::memcpy(allocated_data, image_data.data(), image_data.size());
    
    ASSERT_TRUE(processor.resize(width, height, allocated_data).isSuccess());
    
    // Применяем несколько фильтров
    auto& factory = FilterFactory::getInstance();
    CLI::App app;
    
    // Градации серого
    auto grayscale = factory.create("grayscale", app);
    ASSERT_NE(grayscale, nullptr);
    EXPECT_TRUE(grayscale->apply(processor).isSuccess());
    
    // Размытие
    double blur_radius = 2.0;
    app.add_option("--blur-radius", blur_radius);
    auto blur = factory.create("blur", app);
    ASSERT_NE(blur, nullptr);
    EXPECT_TRUE(blur->apply(processor).isSuccess());
    
    // Повышение резкости
    // Используем значение по умолчанию (1.0), опция не требуется
    auto sharpen = factory.create("sharpen", app);
    ASSERT_NE(sharpen, nullptr);
    EXPECT_TRUE(sharpen->apply(processor).isSuccess());
    
    EXPECT_TRUE(processor.isValid());
}

/**
 * @brief Тест обработки ошибок в полном цикле
 */
TEST_F(IntegrationTestBase, ErrorHandlingInFullCycle)
{
    // Пытаемся загрузить несуществующий файл
    ImageProcessor processor;
    EXPECT_FALSE(processor.loadFromFile("nonexistent_file.jpg").isSuccess());
    
    // Пытаемся применить фильтр к невалидному изображению
    auto& factory = FilterFactory::getInstance();
    CLI::App app;
    auto filter = factory.create("grayscale", app);
    ASSERT_NE(filter, nullptr);
    
    const auto result = filter->apply(processor);
    EXPECT_FALSE(result.isSuccess());
}

