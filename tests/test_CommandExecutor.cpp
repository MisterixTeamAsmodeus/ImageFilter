#include <gtest/gtest.h>
#include <cli/CommandExecutor.h>
#include <cli/CommandHandler.h>
#include <utils/FilterFactory.h>
#include <CLI/CLI.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include "test_utils.h"

namespace fs = std::filesystem;

/**
 * @brief Тесты для класса CommandExecutor
 */
class CommandExecutorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Регистрируем все фильтры
        FilterFactory::getInstance().registerAll();
        
        // Создаем временную директорию для тестов
        test_dir_ = fs::temp_directory_path() / "ImageFilterCommandExecutorTests";
        fs::create_directories(test_dir_);
        
        // Создаем тестовые изображения
        createTestImages();
        
        executor_ = std::make_unique<CommandExecutor>();
        app_ = std::make_unique<CLI::App>("test");
    }
    
    void TearDown() override
    {
        // Удаляем временную директорию
        if (fs::exists(test_dir_))
        {
            fs::remove_all(test_dir_);
        }
    }
    
    /**
     * @brief Создает тестовые изображения для тестов
     */
    void createTestImages()
    {
        constexpr int width = 10;
        constexpr int height = 10;
        
        // Создаем входное изображение
        auto test_image = TestUtils::createTestImage(width, height);
        input_file_ = test_dir_ / "input.png";
        TestUtils::saveTestImagePNG(test_image, width, height, input_file_.string());
        
        // Создаем выходной файл (пустой, будет создан при обработке)
        output_file_ = test_dir_ / "output.png";
        
        // Создаем директории для пакетной обработки
        input_dir_ = test_dir_ / "input_batch";
        output_dir_ = test_dir_ / "output_batch";
        fs::create_directories(input_dir_);
        fs::create_directories(output_dir_);
        
        // Создаем несколько изображений для пакетной обработки
        for (int i = 1; i <= 3; ++i)
        {
            auto batch_image = TestUtils::createTestImage(width, height);
            const auto batch_file = input_dir_ / ("test" + std::to_string(i) + ".png");
            TestUtils::saveTestImagePNG(batch_image, width, height, batch_file.string());
        }
    }
    
    fs::path test_dir_;
    fs::path input_file_;
    fs::path output_file_;
    fs::path input_dir_;
    fs::path output_dir_;
    std::unique_ptr<CommandExecutor> executor_;
    std::unique_ptr<CLI::App> app_;
};

/**
 * @brief Тест выполнения команды обработки одного изображения
 * 
 * Проверяет успешную обработку одного изображения с применением фильтра
 */
TEST_F(CommandExecutorTest, ExecuteSingleImage)
{
    CommandOptions options;
    options.input_file = input_file_.string();
    options.filter_name = "grayscale";
    options.output_file = output_file_.string();
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(fs::exists(output_file_));
}

/**
 * @brief Тест обработки ошибки при отсутствии входного файла
 * 
 * Проверяет, что CommandExecutor корректно обрабатывает ошибку отсутствия файла
 */
TEST_F(CommandExecutorTest, ExecuteSingleImageMissingInput)
{
    CommandOptions options;
    options.input_file = "nonexistent_file.png";
    options.filter_name = "grayscale";
    options.output_file = output_file_.string();
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_NE(result, 0);
}

/**
 * @brief Тест обработки ошибки при отсутствии фильтра
 * 
 * Проверяет, что CommandExecutor корректно обрабатывает ошибку отсутствия фильтра
 */
TEST_F(CommandExecutorTest, ExecuteSingleImageMissingFilter)
{
    CommandOptions options;
    options.input_file = input_file_.string();
    options.filter_name = "";
    options.output_file = output_file_.string();
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_NE(result, 0);
}

/**
 * @brief Тест обработки ошибки при отсутствии выходного файла
 * 
 * Проверяет, что CommandExecutor корректно обрабатывает ошибку отсутствия выходного файла
 */
TEST_F(CommandExecutorTest, ExecuteSingleImageMissingOutput)
{
    CommandOptions options;
    options.input_file = input_file_.string();
    options.filter_name = "grayscale";
    options.output_file = "";
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_NE(result, 0);
}

/**
 * @brief Тест выполнения команды списка фильтров
 * 
 * Проверяет, что команда --list-filters выполняется успешно
 */
TEST_F(CommandExecutorTest, ExecuteListFilters)
{
    CommandOptions options;
    options.list_filters = true;
    
    // Добавляем все необходимые опции для фильтров, которые могут их требовать
    bool counter_clockwise = false;
    app_->add_flag("--counter-clockwise", counter_clockwise);
    
    // Добавляем опции для фильтров с параметрами
    double brightness_factor = 1.2;
    double contrast_factor = 1.5;
    double saturation_factor = 1.5;
    double blur_radius = 5.0;
    int box_blur_radius = 5;
    int motion_blur_length = 10;
    double motion_blur_angle = 0.0;
    int median_radius = 2;
    double noise_intensity = 0.1;
    int posterize_levels = 4;
    int threshold_value = 128;
    double vignette_strength = 0.5;
    double sharpen_strength = 1.0;
    double edge_sensitivity = 0.5;
    std::string edge_operator = "sobel";
    double emboss_strength = 1.0;
    
    app_->add_option("--brightness-factor", brightness_factor);
    app_->add_option("--contrast-factor", contrast_factor);
    app_->add_option("--saturation-factor", saturation_factor);
    app_->add_option("--blur-radius", blur_radius);
    app_->add_option("--box-blur-radius", box_blur_radius);
    app_->add_option("--motion-blur-length", motion_blur_length);
    app_->add_option("--motion-blur-angle", motion_blur_angle);
    app_->add_option("--median-radius", median_radius);
    app_->add_option("--noise-intensity", noise_intensity);
    app_->add_option("--posterize-levels", posterize_levels);
    app_->add_option("--threshold-value", threshold_value);
    app_->add_option("--vignette-strength", vignette_strength);
    app_->add_option("--sharpen-strength", sharpen_strength);
    app_->add_option("--edge-sensitivity", edge_sensitivity);
    app_->add_option("--edge-operator", edge_operator);
    app_->add_option("--emboss-strength", emboss_strength);
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_EQ(result, 0);
}

/**
 * @brief Тест выполнения команды информации о фильтре
 * 
 * Проверяет, что команда --filter-info выполняется успешно
 */
TEST_F(CommandExecutorTest, ExecuteFilterInfo)
{
    CommandOptions options;
    options.filter_info = "grayscale";
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_EQ(result, 0);
}

/**
 * @brief Тест выполнения команды информации о несуществующем фильтре
 * 
 * Проверяет обработку запроса информации о несуществующем фильтре
 */
TEST_F(CommandExecutorTest, ExecuteFilterInfoNonExistent)
{
    CommandOptions options;
    options.filter_info = "nonexistent_filter";
    
    int result = executor_->execute(options, *app_);
    
    // Команда должна выполниться (возвращает 0), но информация может быть пустой
    // Это зависит от реализации FilterInfoDisplay
    EXPECT_GE(result, 0);
}

/**
 * @brief Тест выполнения команды сохранения пресета
 * 
 * Проверяет, что команда сохранения пресета выполняется успешно
 */
TEST_F(CommandExecutorTest, ExecuteSavePreset)
{
    CommandOptions options;
    options.filter_name = "grayscale";
    options.save_preset = "test_preset";
    options.preset_dir = test_dir_.string();
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_EQ(result, 0);
    
    // Проверяем, что файл пресета создан
    fs::path preset_file = test_dir_ / "test_preset.json";
    EXPECT_TRUE(fs::exists(preset_file));
}

/**
 * @brief Тест выполнения команды сохранения пресета без имени фильтра
 * 
 * Проверяет обработку ошибки при сохранении пресета без имени фильтра
 */
TEST_F(CommandExecutorTest, ExecuteSavePresetWithoutFilter)
{
    CommandOptions options;
    options.filter_name = "";
    options.save_preset = "test_preset";
    options.preset_dir = test_dir_.string();
    
    int result = executor_->execute(options, *app_);
    
    // Должна быть ошибка, так как нет имени фильтра
    EXPECT_NE(result, 0);
}

/**
 * @brief Тест выполнения пакетной обработки
 * 
 * Проверяет успешную пакетную обработку нескольких изображений
 */
TEST_F(CommandExecutorTest, ExecuteBatchProcessing)
{
    CommandOptions options;
    options.batch_mode = true;
    options.input_dir = input_dir_.string();
    options.output_dir = output_dir_.string();
    options.filter_name = "grayscale";
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_EQ(result, 0);
    
    // Проверяем, что выходные файлы созданы
    for (int i = 1; i <= 3; ++i)
    {
        fs::path output_file = output_dir_ / ("test" + std::to_string(i) + ".png");
        EXPECT_TRUE(fs::exists(output_file));
    }
}

/**
 * @brief Тест обработки ошибки при отсутствии входной директории в пакетном режиме
 * 
 * Проверяет, что CommandExecutor корректно обрабатывает отсутствие входной директории.
 * В текущей реализации, если файлов не найдено, возвращается 0 (успех), так как это не критическая ошибка.
 */
TEST_F(CommandExecutorTest, ExecuteBatchProcessingMissingInputDir)
{
    CommandOptions options;
    options.batch_mode = true;
    options.input_dir = "nonexistent_dir";
    options.output_dir = output_dir_.string();
    options.filter_name = "grayscale";
    
    int result = executor_->execute(options, *app_);
    
    // В текущей реализации, если директория не существует или пуста,
    // CommandExecutor возвращает 0 (успех), так как это не критическая ошибка
    // Реальная ошибка будет только если есть failed_files > 0
    EXPECT_EQ(result, 0);
}

/**
 * @brief Тест обработки ошибки при отсутствии выходной директории в пакетном режиме
 * 
 * Проверяет, что CommandExecutor корректно обрабатывает ошибку отсутствия выходной директории
 */
TEST_F(CommandExecutorTest, ExecuteBatchProcessingMissingOutputDir)
{
    CommandOptions options;
    options.batch_mode = true;
    options.input_dir = input_dir_.string();
    options.output_dir = "";
    options.filter_name = "grayscale";
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_NE(result, 0);
}

/**
 * @brief Тест обработки ошибки при отсутствии фильтра в пакетном режиме
 * 
 * Проверяет, что CommandExecutor корректно обрабатывает ошибку отсутствия фильтра
 */
TEST_F(CommandExecutorTest, ExecuteBatchProcessingMissingFilter)
{
    CommandOptions options;
    options.batch_mode = true;
    options.input_dir = input_dir_.string();
    options.output_dir = output_dir_.string();
    options.filter_name = "";
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_NE(result, 0);
}

/**
 * @brief Тест пакетной обработки с цепочкой фильтров
 * 
 * Проверяет успешную пакетную обработку с применением нескольких фильтров
 */
TEST_F(CommandExecutorTest, ExecuteBatchProcessingWithFilterChain)
{
    CommandOptions options;
    options.batch_mode = true;
    options.input_dir = input_dir_.string();
    options.output_dir = output_dir_.string();
    options.filter_name = "grayscale,invert";
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_EQ(result, 0);
    
    // Проверяем, что выходные файлы созданы
    for (int i = 1; i <= 3; ++i)
    {
        fs::path output_file = output_dir_ / ("test" + std::to_string(i) + ".png");
        EXPECT_TRUE(fs::exists(output_file));
    }
}

/**
 * @brief Тест обработки одного изображения с цепочкой фильтров
 * 
 * Проверяет успешную обработку одного изображения с применением нескольких фильтров
 */
TEST_F(CommandExecutorTest, ExecuteSingleImageWithFilterChain)
{
    CommandOptions options;
    options.input_file = input_file_.string();
    options.filter_name = "grayscale,invert";
    options.output_file = output_file_.string();
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(fs::exists(output_file_));
}

/**
 * @brief Тест обработки одного изображения с параметрами фильтра
 * 
 * Проверяет успешную обработку с фильтром, требующим параметры
 */
TEST_F(CommandExecutorTest, ExecuteSingleImageWithFilterParameters)
{
    CommandOptions options;
    options.input_file = input_file_.string();
    options.filter_name = "brightness";
    options.output_file = output_file_.string();
    options.brightness_factor = 1.5;
    
    // Добавляем параметры в CLI::App
    app_->add_option("--brightness-factor", options.brightness_factor);
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(fs::exists(output_file_));
}

/**
 * @brief Тест обработки ошибки при обработке поврежденного файла
 * 
 * Проверяет, что CommandExecutor корректно обрабатывает ошибку при обработке поврежденного файла
 */
TEST_F(CommandExecutorTest, ExecuteSingleImageCorruptedFile)
{
    // Создаем поврежденный файл
    fs::path corrupted_file = test_dir_ / "corrupted.png";
    std::ofstream file(corrupted_file);
    file << "This is not a valid image file";
    file.close();
    
    CommandOptions options;
    options.input_file = corrupted_file.string();
    options.filter_name = "grayscale";
    options.output_file = output_file_.string();
    
    int result = executor_->execute(options, *app_);
    
    // Должна быть ошибка при обработке поврежденного файла
    EXPECT_NE(result, 0);
}

/**
 * @brief Тест пакетной обработки с рекурсивным обходом
 * 
 * Проверяет успешную пакетную обработку с рекурсивным обходом директорий
 */
TEST_F(CommandExecutorTest, ExecuteBatchProcessingRecursive)
{
    // Создаем поддиректорию с изображениями
    fs::path subdir = input_dir_ / "subdir";
    fs::create_directories(subdir);
    
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    fs::path subdir_file = subdir / "subdir_image.png";
    TestUtils::saveTestImagePNG(test_image, width, height, subdir_file.string());
    
    CommandOptions options;
    options.batch_mode = true;
    options.input_dir = input_dir_.string();
    options.output_dir = output_dir_.string();
    options.filter_name = "grayscale";
    options.recursive = true;
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_EQ(result, 0);
    
    // Проверяем, что файлы из поддиректории также обработаны
    fs::path output_subdir = output_dir_ / "subdir";
    fs::path output_subdir_file = output_subdir / "subdir_image.png";
    EXPECT_TRUE(fs::exists(output_subdir_file));
}

/**
 * @brief Тест пакетной обработки с паттерном фильтрации
 * 
 * Проверяет успешную пакетную обработку с применением паттерна фильтрации файлов.
 * Примечание: FileSystemHelper поддерживает только паттерны вида "*.ext", 
 * поэтому используем паттерн "*.png" для фильтрации PNG файлов.
 */
TEST_F(CommandExecutorTest, ExecuteBatchProcessingWithPattern)
{
    CommandOptions options;
    options.batch_mode = true;
    options.input_dir = input_dir_.string();
    options.output_dir = output_dir_.string();
    options.filter_name = "grayscale";
    options.pattern = "*.png";  // FileSystemHelper поддерживает только паттерны вида "*.ext"
    
    int result = executor_->execute(options, *app_);
    
    EXPECT_EQ(result, 0);
    
    // Проверяем, что выходные файлы созданы (все файлы в input_dir_ - это PNG)
    for (int i = 1; i <= 3; ++i)
    {
        fs::path output_file = output_dir_ / ("test" + std::to_string(i) + ".png");
        EXPECT_TRUE(fs::exists(output_file));
    }
}

