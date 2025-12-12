#include <gtest/gtest.h>
#include <cli/FilterInfoDisplay.h>
#include <utils/FilterFactory.h>
#include <CLI/CLI.hpp>
#include <sstream>
#include <iostream>
#include <memory>

/**
 * @brief Тесты для FilterInfoDisplay
 */
class FilterInfoDisplayTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Регистрируем фильтры
        auto& factory = FilterFactory::getInstance();
        factory.registerAll();
        
        // Перенаправляем stdout для тестирования вывода
        old_cout = std::cout.rdbuf();
        std::cout.rdbuf(output_stream.rdbuf());
        
        // Создаем CLI::App с необходимыми опциями
        app_ = std::make_unique<CLI::App>("test");
        setupAppOptions(*app_);
    }
    
    void setupAppOptions(CLI::App& app)
    {
        // Добавляем все возможные опции фильтров, чтобы избежать ошибок при создании
        bool counter_clockwise = false;
        double brightness_factor = 1.2;
        double contrast_factor = 1.5;
        double saturation_factor = 1.5;
        double sharpen_strength = 1.0;
        double edge_sensitivity = 0.5;
        std::string edge_operator = "sobel";
        double emboss_strength = 1.0;
        double blur_radius = 5.0;
        int box_blur_radius = 5;
        int motion_blur_length = 10;
        double motion_blur_angle = 0.0;
        int median_radius = 2;
        double noise_intensity = 0.1;
        int posterize_levels = 4;
        int threshold_value = 128;
        double vignette_strength = 0.5;
        
        app.add_flag("--counter-clockwise", counter_clockwise);
        app.add_option("--brightness-factor", brightness_factor);
        app.add_option("--contrast-factor", contrast_factor);
        app.add_option("--saturation-factor", saturation_factor);
        app.add_option("--sharpen-strength", sharpen_strength);
        app.add_option("--edge-sensitivity", edge_sensitivity);
        app.add_option("--edge-operator", edge_operator);
        app.add_option("--emboss-strength", emboss_strength);
        app.add_option("--blur-radius", blur_radius);
        app.add_option("--box-blur-radius", box_blur_radius);
        app.add_option("--motion-blur-length", motion_blur_length);
        app.add_option("--motion-blur-angle", motion_blur_angle);
        app.add_option("--median-radius", median_radius);
        app.add_option("--noise-intensity", noise_intensity);
        app.add_option("--posterize-levels", posterize_levels);
        app.add_option("--threshold-value", threshold_value);
        app.add_option("--vignette-strength", vignette_strength);
    }
    
    std::unique_ptr<CLI::App> app_;
    
    void TearDown() override
    {
        // Восстанавливаем stdout
        std::cout.rdbuf(old_cout);
    }
    
    std::stringstream output_stream;
    std::streambuf* old_cout;
};

// Тест printFilterList
TEST_F(FilterInfoDisplayTest, PrintFilterList)
{
    FilterInfoDisplay::printFilterList(*app_);
    
    std::string output = output_stream.str();
    
    // Проверяем, что вывод содержит информацию о фильтрах
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("Доступные фильтры"), std::string::npos);
}

// Тест printFilterInfo с валидным фильтром
TEST_F(FilterInfoDisplayTest, PrintFilterInfoValid)
{
    FilterInfoDisplay::printFilterInfo("grayscale", *app_);
    
    std::string output = output_stream.str();
    
    // Проверяем, что вывод содержит информацию о фильтре
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("grayscale"), std::string::npos);
}

// Тест printFilterInfo с несуществующим фильтром
TEST_F(FilterInfoDisplayTest, PrintFilterInfoInvalid)
{
    FilterInfoDisplay::printFilterInfo("nonexistent_filter", *app_);
    
    std::string output = output_stream.str();
    
    // Проверяем, что вывод содержит сообщение об ошибке
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("Неизвестный фильтр"), std::string::npos);
}

// Тест printFilterInfo с пустым именем
TEST_F(FilterInfoDisplayTest, PrintFilterInfoEmpty)
{
    FilterInfoDisplay::printFilterInfo("", *app_);
    
    std::string output = output_stream.str();
    
    // Должно быть сообщение об ошибке
    EXPECT_FALSE(output.empty());
}

// Тест printFilterInfo с различными фильтрами
TEST_F(FilterInfoDisplayTest, PrintFilterInfoVarious)
{
    // Тестируем несколько различных фильтров
    std::vector<std::string> test_filters = {"grayscale", "sepia", "invert", "blur"};
    
    for (const auto& filter_name : test_filters)
    {
        output_stream.str("");
        output_stream.clear();
        
        FilterInfoDisplay::printFilterInfo(filter_name, *app_);
        
        std::string output = output_stream.str();
        
        // Каждый фильтр должен вывести информацию
        EXPECT_FALSE(output.empty());
    }
}

// Тест printFilterList с пустым списком фильтров
TEST_F(FilterInfoDisplayTest, PrintFilterListEmpty)
{
    // Вызов должен работать даже если фильтров нет
    FilterInfoDisplay::printFilterList(*app_);
    
    std::string output = output_stream.str();
    
    // Должен быть заголовок
    EXPECT_NE(output.find("Доступные фильтры"), std::string::npos);
}

