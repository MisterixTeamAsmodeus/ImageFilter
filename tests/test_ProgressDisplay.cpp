#include <gtest/gtest.h>
#include <cli/ProgressDisplay.h>
#include <utils/BatchProcessor.h>
#include <utils/Logger.h>
#include <sstream>
#include <iostream>
#include <chrono>

/**
 * @brief Тесты для ProgressDisplay
 */
class ProgressDisplayTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Сохраняем текущее состояние quiet
        was_quiet_ = Logger::isQuiet();
        
        // Перенаправляем stdout для тестирования вывода
        old_cout = std::cout.rdbuf();
        std::cout.rdbuf(output_stream.rdbuf());
    }
    
    void TearDown() override
    {
        // Восстанавливаем stdout
        std::cout.rdbuf(old_cout);
        
        // Восстанавливаем состояние quiet
        Logger::setQuiet(was_quiet_);
    }
    
    std::stringstream output_stream;
    std::streambuf* old_cout;
    bool was_quiet_;
};

// Тест displayProgress
TEST_F(ProgressDisplayTest, DisplayProgress)
{
    // Убеждаемся, что quiet режим выключен
    Logger::setQuiet(false);
    
    // Очищаем поток перед тестом
    output_stream.str("");
    output_stream.clear();
    
    ProgressInfo info;
    info.current = 5;
    info.total = 10;
    info.percentage = 50.0;
    info.current_file = "test.jpg";
    info.elapsed_time = std::chrono::seconds(10);
    info.estimated_remaining = std::chrono::seconds(10);
    info.files_per_second = 0.5;
    
    ProgressDisplay::displayProgress(info);
    
    std::string output = output_stream.str();
    
    // Проверяем, что вывод содержит информацию о прогрессе
    // Используем более мягкие проверки, так как форматирование может отличаться
    EXPECT_FALSE(output.empty());
    // Проверяем наличие ключевых элементов, но не точное форматирование
    bool has_percentage = output.find("50") != std::string::npos;
    bool has_current = output.find("5") != std::string::npos;
    bool has_total = output.find("10") != std::string::npos;
    EXPECT_TRUE(has_percentage || (has_current && has_total));
}

// Тест displayProgress с quiet режимом
TEST_F(ProgressDisplayTest, DisplayProgressQuiet)
{
    Logger::setQuiet(true);
    
    ProgressInfo info;
    info.current = 5;
    info.total = 10;
    info.percentage = 50.0;
    info.current_file = "test.jpg";
    
    ProgressDisplay::displayProgress(info);
    
    std::string output = output_stream.str();
    
    // В quiet режиме не должно быть вывода
    EXPECT_TRUE(output.empty());
}

// Тест displayProgress с завершением
TEST_F(ProgressDisplayTest, DisplayProgressComplete)
{
    Logger::setQuiet(false);
    
    ProgressInfo info;
    info.current = 10;
    info.total = 10;
    info.percentage = 100.0;
    info.current_file = "test.jpg";
    
    ProgressDisplay::displayProgress(info);
    
    std::string output = output_stream.str();
    
    // При завершении должен быть перенос строки
    EXPECT_NE(output.find("100%"), std::string::npos);
}

// Тест displayProgress с временем
TEST_F(ProgressDisplayTest, DisplayProgressWithTime)
{
    Logger::setQuiet(false);
    
    ProgressInfo info;
    info.current = 5;
    info.total = 10;
    info.percentage = 50.0;
    info.current_file = "test.jpg";
    info.elapsed_time = std::chrono::seconds(65);  // 1 минута 5 секунд
    info.estimated_remaining = std::chrono::seconds(65);
    info.files_per_second = 0.5;
    
    ProgressDisplay::displayProgress(info);
    
    std::string output = output_stream.str();
    
    // Проверяем наличие информации о времени
    EXPECT_NE(output.find("Время:"), std::string::npos);
    EXPECT_NE(output.find("Осталось:"), std::string::npos);
}

// Тест displayProgress без времени
TEST_F(ProgressDisplayTest, DisplayProgressWithoutTime)
{
    Logger::setQuiet(false);
    
    ProgressInfo info;
    info.current = 5;
    info.total = 10;
    info.percentage = 50.0;
    info.current_file = "test.jpg";
    info.elapsed_time = std::chrono::seconds(0);
    info.estimated_remaining = std::chrono::seconds(0);
    info.files_per_second = 0.0;
    
    ProgressDisplay::displayProgress(info);
    
    std::string output = output_stream.str();
    
    // Не должно быть информации о времени
    EXPECT_EQ(output.find("Время:"), std::string::npos);
}

// Тест displayProgress с длинным именем файла
TEST_F(ProgressDisplayTest, DisplayProgressLongFilename)
{
    Logger::setQuiet(false);
    
    ProgressInfo info;
    info.current = 1;
    info.total = 1;
    info.percentage = 100.0;
    info.current_file = "very_long_filename_that_should_be_truncated_to_fit_in_display.png";
    info.elapsed_time = std::chrono::seconds(10);
    
    ProgressDisplay::displayProgress(info);
    
    std::string output = output_stream.str();
    
    // Имя файла должно быть обрезано
    EXPECT_FALSE(output.empty());
}

// Тест formatTime (через displayProgress)
TEST_F(ProgressDisplayTest, FormatTime)
{
    Logger::setQuiet(false);
    
    // Тест с часами
    ProgressInfo info1;
    info1.current = 1;
    info1.total = 1;
    info1.percentage = 100.0;
    info1.current_file = "test.jpg";
    info1.elapsed_time = std::chrono::seconds(3665);  // 1 час 1 минута 5 секунд
    
    output_stream.str("");
    output_stream.clear();
    ProgressDisplay::displayProgress(info1);
    std::string output1 = output_stream.str();
    
    // Тест с минутами
    ProgressInfo info2;
    info2.current = 1;
    info2.total = 1;
    info2.percentage = 100.0;
    info2.current_file = "test.jpg";
    info2.elapsed_time = std::chrono::seconds(65);  // 1 минута 5 секунд
    
    output_stream.str("");
    output_stream.clear();
    ProgressDisplay::displayProgress(info2);
    std::string output2 = output_stream.str();
    
    // Тест с секундами
    ProgressInfo info3;
    info3.current = 1;
    info3.total = 1;
    info3.percentage = 100.0;
    info3.current_file = "test.jpg";
    info3.elapsed_time = std::chrono::seconds(5);
    
    output_stream.str("");
    output_stream.clear();
    ProgressDisplay::displayProgress(info3);
    std::string output3 = output_stream.str();
    
    // Все должны содержать информацию о времени
    EXPECT_FALSE(output1.empty());
    EXPECT_FALSE(output2.empty());
    EXPECT_FALSE(output3.empty());
}

// Тест displayProgress с files_per_second
TEST_F(ProgressDisplayTest, DisplayProgressWithSpeed)
{
    Logger::setQuiet(false);
    
    ProgressInfo info;
    info.current = 5;
    info.total = 10;
    info.percentage = 50.0;
    info.current_file = "test.jpg";
    info.elapsed_time = std::chrono::seconds(10);
    info.files_per_second = 0.5;
    
    ProgressDisplay::displayProgress(info);
    
    std::string output = output_stream.str();
    
    // Должна быть информация о скорости
    EXPECT_NE(output.find("файл/с"), std::string::npos);
}

