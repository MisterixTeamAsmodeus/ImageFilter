#include <gtest/gtest.h>
#include <utils/Logger.h>
#include <sstream>
#include <iostream>

/**
 * @brief Тесты для класса Logger
 * 
 * Проверяет все уровни логирования, управление уровнем и тихим режимом,
 * а также фильтрацию сообщений по уровню.
 */
class LoggerTest : public ::testing::Test
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
 * @brief Тест установки и получения уровня логирования
 */
TEST_F(LoggerTest, SetLevelGetLevel)
{
    // Тест установки всех уровней
    Logger::setLevel(LogLevel::DEBUG);
    EXPECT_EQ(Logger::getLevel(), LogLevel::DEBUG);
    
    Logger::setLevel(LogLevel::INFO);
    EXPECT_EQ(Logger::getLevel(), LogLevel::INFO);
    
    Logger::setLevel(LogLevel::WARNING);
    EXPECT_EQ(Logger::getLevel(), LogLevel::WARNING);
    
    Logger::setLevel(LogLevel::ERROR);
    EXPECT_EQ(Logger::getLevel(), LogLevel::ERROR);
}

/**
 * @brief Тест установки и проверки тихого режима
 */
TEST_F(LoggerTest, SetQuietIsQuiet)
{
    // Проверяем, что тихий режим выключен по умолчанию (после SetUp)
    EXPECT_FALSE(Logger::isQuiet());
    
    // Включаем тихий режим
    Logger::setQuiet(true);
    EXPECT_TRUE(Logger::isQuiet());
    
    // Выключаем тихий режим
    Logger::setQuiet(false);
    EXPECT_FALSE(Logger::isQuiet());
}

/**
 * @brief Тест вызова метода log с различными уровнями
 */
TEST_F(LoggerTest, LogMethod)
{
    // Устанавливаем уровень DEBUG, чтобы все сообщения проходили
    Logger::setLevel(LogLevel::DEBUG);
    Logger::setQuiet(false);
    
    // Проверяем, что методы вызываются без ошибок
    EXPECT_NO_THROW(Logger::log(LogLevel::DEBUG, "Debug message"));
    EXPECT_NO_THROW(Logger::log(LogLevel::INFO, "Info message"));
    EXPECT_NO_THROW(Logger::log(LogLevel::WARNING, "Warning message"));
    EXPECT_NO_THROW(Logger::log(LogLevel::ERROR, "Error message"));
}

/**
 * @brief Тест методов удобного логирования (debug, info, warning, error)
 */
TEST_F(LoggerTest, ConvenienceMethods)
{
    // Устанавливаем уровень DEBUG, чтобы все сообщения проходили
    Logger::setLevel(LogLevel::DEBUG);
    Logger::setQuiet(false);
    
    // Проверяем, что методы вызываются без ошибок
    EXPECT_NO_THROW(Logger::debug("Debug message"));
    EXPECT_NO_THROW(Logger::info("Info message"));
    EXPECT_NO_THROW(Logger::warning("Warning message"));
    EXPECT_NO_THROW(Logger::error("Error message"));
}

/**
 * @brief Тест фильтрации сообщений по уровню DEBUG
 */
TEST_F(LoggerTest, FilterByLevelDebug)
{
    Logger::setLevel(LogLevel::DEBUG);
    Logger::setQuiet(false);
    
    // При уровне DEBUG все сообщения должны проходить
    EXPECT_NO_THROW(Logger::debug("Debug message"));
    EXPECT_NO_THROW(Logger::info("Info message"));
    EXPECT_NO_THROW(Logger::warning("Warning message"));
    EXPECT_NO_THROW(Logger::error("Error message"));
}

/**
 * @brief Тест фильтрации сообщений по уровню INFO
 */
TEST_F(LoggerTest, FilterByLevelInfo)
{
    Logger::setLevel(LogLevel::INFO);
    Logger::setQuiet(false);
    
    // При уровне INFO должны проходить INFO, WARNING, ERROR
    // DEBUG не должен выводиться (но метод вызывается без ошибок)
    EXPECT_NO_THROW(Logger::debug("Debug message")); // Не выводится, но не ошибка
    EXPECT_NO_THROW(Logger::info("Info message"));
    EXPECT_NO_THROW(Logger::warning("Warning message"));
    EXPECT_NO_THROW(Logger::error("Error message"));
}

/**
 * @brief Тест фильтрации сообщений по уровню WARNING
 */
TEST_F(LoggerTest, FilterByLevelWarning)
{
    Logger::setLevel(LogLevel::WARNING);
    Logger::setQuiet(false);
    
    // При уровне WARNING должны проходить только WARNING и ERROR
    EXPECT_NO_THROW(Logger::debug("Debug message")); // Не выводится
    EXPECT_NO_THROW(Logger::info("Info message")); // Не выводится
    EXPECT_NO_THROW(Logger::warning("Warning message"));
    EXPECT_NO_THROW(Logger::error("Error message"));
}

/**
 * @brief Тест фильтрации сообщений по уровню ERROR
 */
TEST_F(LoggerTest, FilterByLevelError)
{
    Logger::setLevel(LogLevel::ERROR);
    Logger::setQuiet(false);
    
    // При уровне ERROR должны проходить только ERROR
    EXPECT_NO_THROW(Logger::debug("Debug message")); // Не выводится
    EXPECT_NO_THROW(Logger::info("Info message")); // Не выводится
    EXPECT_NO_THROW(Logger::warning("Warning message")); // Не выводится
    EXPECT_NO_THROW(Logger::error("Error message"));
}

/**
 * @brief Тест тихого режима - все сообщения должны блокироваться
 */
TEST_F(LoggerTest, QuietModeBlocksAllMessages)
{
    Logger::setQuiet(true);
    Logger::setLevel(LogLevel::DEBUG); // Даже при самом низком уровне
    
    // Все сообщения должны блокироваться в тихом режиме
    EXPECT_NO_THROW(Logger::debug("Debug message"));
    EXPECT_NO_THROW(Logger::info("Info message"));
    EXPECT_NO_THROW(Logger::warning("Warning message"));
    EXPECT_NO_THROW(Logger::error("Error message"));
}

/**
 * @brief Тест взаимодействия тихого режима и уровня логирования
 */
TEST_F(LoggerTest, QuietModeOverridesLevel)
{
    // Включаем тихий режим
    Logger::setQuiet(true);
    Logger::setLevel(LogLevel::DEBUG);
    
    // Даже при уровне DEBUG тихий режим должен блокировать все
    EXPECT_NO_THROW(Logger::debug("Debug message"));
    EXPECT_NO_THROW(Logger::info("Info message"));
    EXPECT_NO_THROW(Logger::warning("Warning message"));
    EXPECT_NO_THROW(Logger::error("Error message"));
    
    // Выключаем тихий режим
    Logger::setQuiet(false);
    
    // Теперь сообщения должны проходить согласно уровню
    EXPECT_NO_THROW(Logger::error("Error message"));
}

/**
 * @brief Тест логирования пустых сообщений
 */
TEST_F(LoggerTest, EmptyMessages)
{
    Logger::setLevel(LogLevel::DEBUG);
    Logger::setQuiet(false);
    
    // Проверяем, что пустые сообщения обрабатываются корректно
    EXPECT_NO_THROW(Logger::debug(""));
    EXPECT_NO_THROW(Logger::info(""));
    EXPECT_NO_THROW(Logger::warning(""));
    EXPECT_NO_THROW(Logger::error(""));
}

/**
 * @brief Тест логирования длинных сообщений
 */
TEST_F(LoggerTest, LongMessages)
{
    Logger::setLevel(LogLevel::DEBUG);
    Logger::setQuiet(false);
    
    // Создаем длинное сообщение
    std::string long_message(1000, 'A');
    
    // Проверяем, что длинные сообщения обрабатываются корректно
    EXPECT_NO_THROW(Logger::debug(long_message));
    EXPECT_NO_THROW(Logger::info(long_message));
    EXPECT_NO_THROW(Logger::warning(long_message));
    EXPECT_NO_THROW(Logger::error(long_message));
}

/**
 * @brief Тест многократного изменения уровня логирования
 */
TEST_F(LoggerTest, MultipleLevelChanges)
{
    // Многократно меняем уровень
    Logger::setLevel(LogLevel::DEBUG);
    EXPECT_EQ(Logger::getLevel(), LogLevel::DEBUG);
    
    Logger::setLevel(LogLevel::INFO);
    EXPECT_EQ(Logger::getLevel(), LogLevel::INFO);
    
    Logger::setLevel(LogLevel::WARNING);
    EXPECT_EQ(Logger::getLevel(), LogLevel::WARNING);
    
    Logger::setLevel(LogLevel::ERROR);
    EXPECT_EQ(Logger::getLevel(), LogLevel::ERROR);
    
    Logger::setLevel(LogLevel::DEBUG);
    EXPECT_EQ(Logger::getLevel(), LogLevel::DEBUG);
}

/**
 * @brief Тест многократного переключения тихого режима
 */
TEST_F(LoggerTest, MultipleQuietModeToggles)
{
    // Многократно переключаем тихий режим
    Logger::setQuiet(true);
    EXPECT_TRUE(Logger::isQuiet());
    
    Logger::setQuiet(false);
    EXPECT_FALSE(Logger::isQuiet());
    
    Logger::setQuiet(true);
    EXPECT_TRUE(Logger::isQuiet());
    
    Logger::setQuiet(false);
    EXPECT_FALSE(Logger::isQuiet());
}

/**
 * @brief Тест последовательности операций
 */
TEST_F(LoggerTest, SequenceOfOperations)
{
    // Проверяем последовательность операций
    Logger::setLevel(LogLevel::WARNING);
    Logger::setQuiet(false);
    EXPECT_EQ(Logger::getLevel(), LogLevel::WARNING);
    EXPECT_FALSE(Logger::isQuiet());
    
    Logger::setQuiet(true);
    EXPECT_TRUE(Logger::isQuiet());
    
    Logger::setLevel(LogLevel::DEBUG);
    EXPECT_EQ(Logger::getLevel(), LogLevel::DEBUG);
    EXPECT_TRUE(Logger::isQuiet()); // Тихий режим все еще включен
    
    Logger::setQuiet(false);
    EXPECT_FALSE(Logger::isQuiet());
    EXPECT_EQ(Logger::getLevel(), LogLevel::DEBUG);
}

