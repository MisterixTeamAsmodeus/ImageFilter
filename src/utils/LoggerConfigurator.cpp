#include <utils/LoggerConfigurator.h>

void LoggerConfigurator::configure(const CommandOptions& options)
{
    if (options.quiet)
    {
        Logger::setQuiet(true);
    }
    else
    {
        Logger::setQuiet(false);
        if (options.log_level_str == "DEBUG")
        {
            Logger::setLevel(LogLevel::DEBUG);
        }
        else if (options.log_level_str == "INFO")
        {
            Logger::setLevel(LogLevel::INFO);
        }
        else if (options.log_level_str == "WARNING")
        {
            Logger::setLevel(LogLevel::WARNING);
        }
        else if (options.log_level_str == "ERROR")
        {
            Logger::setLevel(LogLevel::ERROR);
        }
    }
}

