#include <gtest/gtest.h>
#include <cli/CommandParser.h>
#include <cli/CommandHandler.h>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

/**
 * @brief Тесты для класса CommandParser
 */
class CommandParserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        parser_ = std::make_unique<CommandParser>();
    }
    
    void TearDown() override
    {
        parser_.reset();
    }
    
    /**
     * @brief Вспомогательная функция для создания аргументов командной строки
     */
    std::vector<std::string> createArgs(const std::vector<std::string>& args)
    {
        std::vector<std::string> result;
        result.push_back("ImageFilter"); // Имя программы
        result.insert(result.end(), args.begin(), args.end());
        return result;
    }
    
    /**
     * @brief Вспомогательная функция для преобразования вектора строк в char**
     */
    std::vector<char*> createArgv(const std::vector<std::string>& args)
    {
        std::vector<char*> argv;
        for (auto& arg : args)
        {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);
        return argv;
    }
    
    std::unique_ptr<CommandParser> parser_;
};

/**
 * @brief Тест парсинга базовых параметров
 */
TEST_F(CommandParserTest, ParseBasicOptions)
{
    CommandOptions options;
    
    const auto args = createArgs({"input.jpg", "grayscale", "output.jpg"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(options.input_file, "input.jpg");
    EXPECT_EQ(options.filter_name, "grayscale");
    EXPECT_EQ(options.output_file, "output.jpg");
}

/**
 * @brief Тест парсинга флага --list-filters
 */
TEST_F(CommandParserTest, ParseListFilters)
{
    CommandOptions options;
    
    const auto args = createArgs({"--list-filters"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(options.list_filters);
}

/**
 * @brief Тест парсинга опции --filter-info
 */
TEST_F(CommandParserTest, ParseFilterInfo)
{
    CommandOptions options;
    
    const auto args = createArgs({"--filter-info", "grayscale"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(options.filter_info, "grayscale");
}

/**
 * @brief Тест парсинга флага --quiet
 */
TEST_F(CommandParserTest, ParseQuiet)
{
    CommandOptions options;
    
    const auto args = createArgs({"-q"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(options.quiet);
}

/**
 * @brief Тест парсинга опции --log-level
 */
TEST_F(CommandParserTest, ParseLogLevel)
{
    CommandOptions options;
    
    const auto args = createArgs({"--log-level", "DEBUG"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(options.log_level_str, "DEBUG");
}

/**
 * @brief Тест парсинга флага --preserve-alpha
 */
TEST_F(CommandParserTest, ParsePreserveAlpha)
{
    CommandOptions options;
    
    const auto args = createArgs({"--preserve-alpha"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(options.preserve_alpha);
}

/**
 * @brief Тест парсинга флага --force-rgb
 */
TEST_F(CommandParserTest, ParseForceRgb)
{
    CommandOptions options;
    
    const auto args = createArgs({"--force-rgb"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(options.force_rgb);
}

/**
 * @brief Тест парсинга опции --jpeg-quality
 */
TEST_F(CommandParserTest, ParseJpegQuality)
{
    CommandOptions options;
    
    const auto args = createArgs({"--jpeg-quality", "95"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(options.jpeg_quality, 95);
}

/**
 * @brief Тест парсинга опции --preset
 */
TEST_F(CommandParserTest, ParsePreset)
{
    CommandOptions options;
    
    const auto args = createArgs({"--preset", "vintage.json"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(options.preset_file, "vintage.json");
}

/**
 * @brief Тест парсинга опции --save-preset
 */
TEST_F(CommandParserTest, ParseSavePreset)
{
    CommandOptions options;
    
    const auto args = createArgs({"--save-preset", "my_preset.json"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(options.save_preset, "my_preset.json");
}

/**
 * @brief Тест парсинга опции --preset-dir
 */
TEST_F(CommandParserTest, ParsePresetDir)
{
    CommandOptions options;
    
    const auto args = createArgs({"--preset-dir", "./presets"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(options.preset_dir, "./presets");
}

/**
 * @brief Тест парсинга флага --batch
 */
TEST_F(CommandParserTest, ParseBatch)
{
    CommandOptions options;
    
    const auto args = createArgs({"--batch"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(options.batch_mode);
}

/**
 * @brief Тест парсинга опций пакетной обработки
 */
TEST_F(CommandParserTest, ParseBatchOptions)
{
    CommandOptions options;
    
    const auto args = createArgs({
        "--batch",
        "--input-dir", "./input",
        "--output-dir", "./output",
        "--recursive",
        "--pattern", "*.jpg"
    });
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(options.batch_mode);
    EXPECT_EQ(options.input_dir, "./input");
    EXPECT_EQ(options.output_dir, "./output");
    EXPECT_TRUE(options.recursive);
    EXPECT_EQ(options.pattern, "*.jpg");
}

/**
 * @brief Тест парсинга опции --resume-state
 */
TEST_F(CommandParserTest, ParseResumeState)
{
    CommandOptions options;
    
    const auto args = createArgs({"--resume-state", "state.json"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(options.resume_state_file, "state.json");
}

/**
 * @brief Тест парсинга параметров фильтров - brightness
 */
TEST_F(CommandParserTest, ParseBrightnessFactor)
{
    CommandOptions options;
    
    const auto args = createArgs({"--brightness-factor", "1.5"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_DOUBLE_EQ(options.brightness_factor, 1.5);
}

/**
 * @brief Тест парсинга параметров фильтров - contrast
 */
TEST_F(CommandParserTest, ParseContrastFactor)
{
    CommandOptions options;
    
    const auto args = createArgs({"--contrast-factor", "2.0"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_DOUBLE_EQ(options.contrast_factor, 2.0);
}

/**
 * @brief Тест парсинга параметров фильтров - saturation
 */
TEST_F(CommandParserTest, ParseSaturationFactor)
{
    CommandOptions options;
    
    const auto args = createArgs({"--saturation-factor", "1.8"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_DOUBLE_EQ(options.saturation_factor, 1.8);
}

/**
 * @brief Тест парсинга флага --counter-clockwise
 */
TEST_F(CommandParserTest, ParseCounterClockwise)
{
    CommandOptions options;
    
    const auto args = createArgs({"--counter-clockwise"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(options.counter_clockwise);
}

/**
 * @brief Тест парсинга параметров размытия
 */
TEST_F(CommandParserTest, ParseBlurOptions)
{
    CommandOptions options;
    
    const auto args = createArgs({
        "--blur-radius", "7.5",
        "--box-blur-radius", "10",
        "--motion-blur-length", "15",
        "--motion-blur-angle", "45.0"
    });
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_DOUBLE_EQ(options.blur_radius, 7.5);
    EXPECT_EQ(options.box_blur_radius, 10);
    EXPECT_EQ(options.motion_blur_length, 15);
    EXPECT_DOUBLE_EQ(options.motion_blur_angle, 45.0);
}

/**
 * @brief Тест парсинга параметров медианного фильтра
 */
TEST_F(CommandParserTest, ParseMedianRadius)
{
    CommandOptions options;
    
    const auto args = createArgs({"--median-radius", "3"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(options.median_radius, 3);
}

/**
 * @brief Тест парсинга параметров шума
 */
TEST_F(CommandParserTest, ParseNoiseIntensity)
{
    CommandOptions options;
    
    const auto args = createArgs({"--noise-intensity", "0.2"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_DOUBLE_EQ(options.noise_intensity, 0.2);
}

/**
 * @brief Тест парсинга параметров постеризации
 */
TEST_F(CommandParserTest, ParsePosterizeLevels)
{
    CommandOptions options;
    
    const auto args = createArgs({"--posterize-levels", "8"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(options.posterize_levels, 8);
}

/**
 * @brief Тест парсинга параметров порога
 */
TEST_F(CommandParserTest, ParseThresholdValue)
{
    CommandOptions options;
    
    const auto args = createArgs({"--threshold-value", "200"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(options.threshold_value, 200);
}

/**
 * @brief Тест парсинга параметров виньетирования
 */
TEST_F(CommandParserTest, ParseVignetteStrength)
{
    CommandOptions options;
    
    const auto args = createArgs({"--vignette-strength", "0.8"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_DOUBLE_EQ(options.vignette_strength, 0.8);
}

/**
 * @brief Тест парсинга параметров резкости
 */
TEST_F(CommandParserTest, ParseSharpenStrength)
{
    CommandOptions options;
    
    const auto args = createArgs({"--sharpen-strength", "2.0"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_DOUBLE_EQ(options.sharpen_strength, 2.0);
}

/**
 * @brief Тест парсинга параметров детекции краев
 */
TEST_F(CommandParserTest, ParseEdgeDetectionOptions)
{
    CommandOptions options;
    
    const auto args = createArgs({
        "--edge-sensitivity", "0.7",
        "--edge-operator", "prewitt"
    });
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_DOUBLE_EQ(options.edge_sensitivity, 0.7);
    EXPECT_EQ(options.edge_operator, "prewitt");
}

/**
 * @brief Тест парсинга параметров рельефа
 */
TEST_F(CommandParserTest, ParseEmbossStrength)
{
    CommandOptions options;
    
    const auto args = createArgs({"--emboss-strength", "1.5"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_DOUBLE_EQ(options.emboss_strength, 1.5);
}

/**
 * @brief Тест парсинга комбинированных опций
 */
TEST_F(CommandParserTest, ParseCombinedOptions)
{
    CommandOptions options;
    
    const auto args = createArgs({
        "input.jpg",
        "grayscale,sepia",
        "output.jpg",
        "--quiet",
        "--log-level", "WARNING",
        "--jpeg-quality", "95",
        "--brightness-factor", "1.3"
    });
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(options.input_file, "input.jpg");
    EXPECT_EQ(options.filter_name, "grayscale,sepia");
    EXPECT_EQ(options.output_file, "output.jpg");
    EXPECT_TRUE(options.quiet);
    EXPECT_EQ(options.log_level_str, "WARNING");
    EXPECT_EQ(options.jpeg_quality, 95);
    EXPECT_DOUBLE_EQ(options.brightness_factor, 1.3);
}

/**
 * @brief Тест парсинга с несуществующей опцией
 */
TEST_F(CommandParserTest, ParseInvalidOption)
{
    CommandOptions options;
    
    const auto args = createArgs({"--nonexistent-option", "value"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    // CLI11 должен вернуть ошибку для несуществующей опции
    EXPECT_NE(result, 0);
}

/**
 * @brief Тест парсинга с отсутствующим значением опции
 */
TEST_F(CommandParserTest, ParseMissingOptionValue)
{
    CommandOptions options;
    
    const auto args = createArgs({"--log-level"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    // CLI11 должен вернуть ошибку для отсутствующего значения
    EXPECT_NE(result, 0);
}

/**
 * @brief Тест парсинга пустых аргументов
 */
TEST_F(CommandParserTest, ParseEmptyArguments)
{
    CommandOptions options;
    
    const auto args = createArgs({});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    // Парсинг должен пройти успешно (все опции необязательны)
    EXPECT_EQ(result, 0);
}

/**
 * @brief Тест получения CLI::App
 */
TEST_F(CommandParserTest, GetApp)
{
    CLI::App& app = parser_->getApp();
    
    // Проверяем, что можем получить доступ к приложению
    EXPECT_NE(&app, nullptr);
}

/**
 * @brief Тест парсинга цепочки фильтров через запятую
 */
TEST_F(CommandParserTest, ParseFilterChain)
{
    CommandOptions options;
    
    const auto args = createArgs({"input.jpg", "grayscale,sepia,invert", "output.jpg"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(options.filter_name, "grayscale,sepia,invert");
}

/**
 * @brief Тест парсинга с различными операторами детекции краев
 * 
 * Примечание: CLI11 не позволяет вызывать parse() несколько раз на одном объекте,
 * поэтому проверяем только один оператор. Другие операторы проверены в ParseEdgeDetectionOptions.
 */
TEST_F(CommandParserTest, ParseEdgeOperators)
{
    CommandOptions options;
    
    const auto args = createArgs({"--edge-operator", "scharr"});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(options.edge_operator, "scharr");
}

/**
 * @brief Тест значений по умолчанию
 */
TEST_F(CommandParserTest, DefaultValues)
{
    CommandOptions options;
    
    const auto args = createArgs({});
    auto argv = createArgv(args);
    
    const int result = parser_->parse(static_cast<int>(argv.size() - 1), argv.data(), options);
    
    EXPECT_EQ(result, 0);
    EXPECT_FALSE(options.list_filters);
    EXPECT_FALSE(options.quiet);
    EXPECT_EQ(options.log_level_str, "INFO");
    EXPECT_FALSE(options.preserve_alpha);
    EXPECT_FALSE(options.force_rgb);
    EXPECT_EQ(options.jpeg_quality, 90);
    EXPECT_FALSE(options.batch_mode);
    EXPECT_FALSE(options.recursive);
    EXPECT_DOUBLE_EQ(options.brightness_factor, 1.2);
    EXPECT_DOUBLE_EQ(options.contrast_factor, 1.5);
    EXPECT_DOUBLE_EQ(options.saturation_factor, 1.5);
    EXPECT_FALSE(options.counter_clockwise);
    EXPECT_DOUBLE_EQ(options.blur_radius, 5.0);
    EXPECT_EQ(options.box_blur_radius, 5);
    EXPECT_EQ(options.motion_blur_length, 10);
    EXPECT_DOUBLE_EQ(options.motion_blur_angle, 0.0);
    EXPECT_EQ(options.median_radius, 2);
    EXPECT_DOUBLE_EQ(options.noise_intensity, 0.1);
    EXPECT_EQ(options.posterize_levels, 4);
    EXPECT_EQ(options.threshold_value, 128);
    EXPECT_DOUBLE_EQ(options.vignette_strength, 0.5);
    EXPECT_DOUBLE_EQ(options.sharpen_strength, 1.0);
    EXPECT_DOUBLE_EQ(options.edge_sensitivity, 0.5);
    EXPECT_EQ(options.edge_operator, "sobel");
    EXPECT_DOUBLE_EQ(options.emboss_strength, 1.0);
}

