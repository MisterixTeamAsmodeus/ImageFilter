#include <gtest/gtest.h>
#include <utils/LoggerConfigurator.h>
#include <utils/Logger.h>
#include <cli/CommandHandler.h>

/**
 * @brief Тесты для класса LoggerConfigurator
 * 
 * Проверяет конфигурацию логирования на основе параметров командной строки.
 */
class LoggerConfiguratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Сохраняем исходное состояние
        original_level_ = Logger::getLevel();
        original_quiet_ = Logger::isQuiet();
        
        // Сбрасываем в известное состояние для каждого теста
        Logger::setQuiet(false);
        Logger::setLevel(LogLevel::INFO);
    }
    
    void TearDown() override
    {
        // Восстанавливаем исходное состояние
        Logger::setLevel(original_level_);
        Logger::setQuiet(original_quiet_);
    }
    
    LogLevel original_level_;
    bool original_quiet_;
};

/**
 * @brief Тест конфигурации с уровнем DEBUG
 */
TEST_F(LoggerConfiguratorTest, ConfigureDebugLevel)
{
    CommandOptions options;
    options.quiet = false;
    options.log_level_str = "DEBUG";
    
    LoggerConfigurator::configure(options);
    
    EXPECT_EQ(Logger::getLevel(), LogLevel::DEBUG);
    EXPECT_FALSE(Logger::isQuiet());
}

/**
 * @brief Тест конфигурации с уровнем INFO
 */
TEST_F(LoggerConfiguratorTest, ConfigureInfoLevel)
{
    CommandOptions options;
    options.quiet = false;
    options.log_level_str = "INFO";
    
    LoggerConfigurator::configure(options);
    
    EXPECT_EQ(Logger::getLevel(), LogLevel::INFO);
    EXPECT_FALSE(Logger::isQuiet());
}

/**
 * @brief Тест конфигурации с уровнем WARNING
 */
TEST_F(LoggerConfiguratorTest, ConfigureWarningLevel)
{
    CommandOptions options;
    options.quiet = false;
    options.log_level_str = "WARNING";
    
    LoggerConfigurator::configure(options);
    
    EXPECT_EQ(Logger::getLevel(), LogLevel::WARNING);
    EXPECT_FALSE(Logger::isQuiet());
}

/**
 * @brief Тест конфигурации с уровнем ERROR
 */
TEST_F(LoggerConfiguratorTest, ConfigureErrorLevel)
{
    CommandOptions options;
    options.quiet = false;
    options.log_level_str = "ERROR";
    
    LoggerConfigurator::configure(options);
    
    EXPECT_EQ(Logger::getLevel(), LogLevel::ERROR);
    EXPECT_FALSE(Logger::isQuiet());
}

/**
 * @brief Тест конфигурации с тихим режимом
 */
TEST_F(LoggerConfiguratorTest, ConfigureQuietMode)
{
    CommandOptions options;
    options.quiet = true;
    options.log_level_str = "DEBUG"; // Даже при DEBUG тихий режим должен включиться
    
    LoggerConfigurator::configure(options);
    
    EXPECT_TRUE(Logger::isQuiet());
    // Уровень не должен измениться, если quiet = true
}

/**
 * @brief Тест конфигурации с тихим режимом и различными уровнями
 */
TEST_F(LoggerConfiguratorTest, ConfigureQuietModeWithDifferentLevels)
{
    // Проверяем, что тихий режим имеет приоритет над уровнем
    CommandOptions options1;
    options1.quiet = true;
    options1.log_level_str = "DEBUG";
    LoggerConfigurator::configure(options1);
    EXPECT_TRUE(Logger::isQuiet());
    
    CommandOptions options2;
    options2.quiet = true;
    options2.log_level_str = "INFO";
    LoggerConfigurator::configure(options2);
    EXPECT_TRUE(Logger::isQuiet());
    
    CommandOptions options3;
    options3.quiet = true;
    options3.log_level_str = "WARNING";
    LoggerConfigurator::configure(options3);
    EXPECT_TRUE(Logger::isQuiet());
    
    CommandOptions options4;
    options4.quiet = true;
    options4.log_level_str = "ERROR";
    LoggerConfigurator::configure(options4);
    EXPECT_TRUE(Logger::isQuiet());
}

/**
 * @brief Тест конфигурации с неизвестным уровнем
 */
TEST_F(LoggerConfiguratorTest, ConfigureUnknownLevel)
{
    CommandOptions options;
    options.quiet = false;
    options.log_level_str = "UNKNOWN_LEVEL";
    
    // Неизвестный уровень не должен изменять текущий уровень
    Logger::setLevel(LogLevel::INFO);
    LoggerConfigurator::configure(options);
    
    // Уровень должен остаться прежним
    EXPECT_EQ(Logger::getLevel(), LogLevel::INFO);
    EXPECT_FALSE(Logger::isQuiet());
}

/**
 * @brief Тест конфигурации с пустой строкой уровня
 */
TEST_F(LoggerConfiguratorTest, ConfigureEmptyLevel)
{
    CommandOptions options;
    options.quiet = false;
    options.log_level_str = "";
    
    Logger::setLevel(LogLevel::WARNING);
    LoggerConfigurator::configure(options);
    
    // Уровень должен остаться прежним
    EXPECT_EQ(Logger::getLevel(), LogLevel::WARNING);
    EXPECT_FALSE(Logger::isQuiet());
}

/**
 * @brief Тест конфигурации с регистронезависимыми уровнями (должны быть регистрозависимыми)
 */
TEST_F(LoggerConfiguratorTest, ConfigureCaseSensitiveLevels)
{
    // Проверяем, что уровни регистрозависимы
    CommandOptions options;
    options.quiet = false;
    options.log_level_str = "debug"; // Строчные буквы
    
    Logger::setLevel(LogLevel::INFO);
    LoggerConfigurator::configure(options);
    
    // Должен остаться прежний уровень, так как "debug" != "DEBUG"
    EXPECT_EQ(Logger::getLevel(), LogLevel::INFO);
}

/**
 * @brief Тест последовательной конфигурации
 */
TEST_F(LoggerConfiguratorTest, SequentialConfiguration)
{
    // Первая конфигурация
    CommandOptions options1;
    options1.quiet = false;
    options1.log_level_str = "DEBUG";
    LoggerConfigurator::configure(options1);
    EXPECT_EQ(Logger::getLevel(), LogLevel::DEBUG);
    EXPECT_FALSE(Logger::isQuiet());
    
    // Вторая конфигурация
    CommandOptions options2;
    options2.quiet = false;
    options2.log_level_str = "WARNING";
    LoggerConfigurator::configure(options2);
    EXPECT_EQ(Logger::getLevel(), LogLevel::WARNING);
    EXPECT_FALSE(Logger::isQuiet());
    
    // Третья конфигурация с тихим режимом
    CommandOptions options3;
    options3.quiet = true;
    options3.log_level_str = "ERROR";
    LoggerConfigurator::configure(options3);
    EXPECT_TRUE(Logger::isQuiet());
    
    // Четвертая конфигурация - выключаем тихий режим
    CommandOptions options4;
    options4.quiet = false;
    options4.log_level_str = "INFO";
    LoggerConfigurator::configure(options4);
    EXPECT_EQ(Logger::getLevel(), LogLevel::INFO);
    EXPECT_FALSE(Logger::isQuiet());
}

/**
 * @brief Тест конфигурации с полным набором опций
 */
TEST_F(LoggerConfiguratorTest, ConfigureWithFullOptions)
{
    CommandOptions options;
    options.quiet = false;
    options.log_level_str = "ERROR";
    options.input_file = "test.jpg";
    options.output_file = "output.jpg";
    options.filter_name = "grayscale";
    // Другие опции не должны влиять на логирование
    
    LoggerConfigurator::configure(options);
    
    EXPECT_EQ(Logger::getLevel(), LogLevel::ERROR);
    EXPECT_FALSE(Logger::isQuiet());
}

/**
 * @brief Тест конфигурации после изменения уровня вручную
 */
TEST_F(LoggerConfiguratorTest, ConfigureAfterManualLevelChange)
{
    // Меняем уровень вручную
    Logger::setLevel(LogLevel::DEBUG);
    EXPECT_EQ(Logger::getLevel(), LogLevel::DEBUG);
    
    // Конфигурируем через LoggerConfigurator
    CommandOptions options;
    options.quiet = false;
    options.log_level_str = "WARNING";
    LoggerConfigurator::configure(options);
    
    // Уровень должен измениться
    EXPECT_EQ(Logger::getLevel(), LogLevel::WARNING);
}

/**
 * @brief Тест конфигурации после включения тихого режима вручную
 */
TEST_F(LoggerConfiguratorTest, ConfigureAfterManualQuietChange)
{
    // Включаем тихий режим вручную
    Logger::setQuiet(true);
    EXPECT_TRUE(Logger::isQuiet());
    
    // Конфигурируем через LoggerConfigurator с quiet = false
    CommandOptions options;
    options.quiet = false;
    options.log_level_str = "INFO";
    LoggerConfigurator::configure(options);
    
    // Тихий режим должен выключиться
    EXPECT_FALSE(Logger::isQuiet());
    EXPECT_EQ(Logger::getLevel(), LogLevel::INFO);
}

/**
 * @brief Тест конфигурации с пробелами в строке уровня
 */
TEST_F(LoggerConfiguratorTest, ConfigureWithWhitespaceInLevel)
{
    CommandOptions options;
    options.quiet = false;
    options.log_level_str = "  INFO  "; // С пробелами
    
    Logger::setLevel(LogLevel::DEBUG);
    LoggerConfigurator::configure(options);
    
    // Уровень не должен измениться из-за пробелов
    EXPECT_EQ(Logger::getLevel(), LogLevel::DEBUG);
}

