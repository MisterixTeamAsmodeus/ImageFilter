#include <utils/Logger.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace {
    /**
     * @brief Преобразует уровень логирования в строку
     * @param level Уровень логирования
     * @return Строковое представление уровня
     */
    const char* levelToString(LogLevel level) noexcept
    {
        switch (level)
        {
            case LogLevel::DEBUG:
                return "DEBUG";
            case LogLevel::INFO:
                return "INFO";
            case LogLevel::WARNING:
                return "WARNING";
            case LogLevel::ERROR:
                return "ERROR";
        }
        __builtin_unreachable();
    }

    /**
     * @brief Получает ссылку на минимальный уровень логирования
     * Использует локальную статическую переменную для избежания exit-time destructors
     * @return Ссылка на минимальный уровень логирования
     */
    LogLevel& getMinLevel() noexcept
    {
        static LogLevel min_level = LogLevel::INFO;
        return min_level;
    }

    /**
     * @brief Получает ссылку на флаг тихого режима
     * Использует локальную статическую переменную для избежания exit-time destructors
     * @return Ссылка на флаг тихого режима
     */
    bool& getQuiet() noexcept
    {
        static bool quiet = false;
        return quiet;
    }
}

void Logger::setLevel(LogLevel level) noexcept
{
    getMinLevel() = level;
}

void Logger::setQuiet(bool quiet) noexcept
{
    getQuiet() = quiet;
}

LogLevel Logger::getLevel() noexcept
{
    return getMinLevel();
}

bool Logger::isQuiet() noexcept
{
    return getQuiet();
}

void Logger::log(LogLevel level, const std::string& message)
{
    if (getQuiet() || level < getMinLevel())
    {
        return;
    }

    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    const auto tm = *std::localtime(&time);

    std::ostream& stream = (level >= LogLevel::ERROR) ? std::cerr : std::cout;

    stream << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] "
           << "[" << levelToString(level) << "] "
           << message << std::endl;
}

void Logger::debug(const std::string& message)
{
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message)
{
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message)
{
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message)
{
    log(LogLevel::ERROR, message);
}
