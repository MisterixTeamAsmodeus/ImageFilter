#include <utils/LoggerConfigurator.h>

#include "utils/Logger.h"

void LoggerConfigurator::configure(bool quiet, const std::string& log_level)
{
    if (quiet)
    {
        Logger::setQuiet(true);
        return;
    }

    Logger::setQuiet(false);

    if (log_level == "DEBUG")
    {
        Logger::setLevel(LogLevel::DEBUG);
    }
    else if (log_level == "INFO")
    {
        Logger::setLevel(LogLevel::INFO);
    }
    else if (log_level == "WARNING")
    {
        Logger::setLevel(LogLevel::WARNING);
    }
    else if (log_level == "ERROR")
    {
        Logger::setLevel(LogLevel::ERROR);
    }
    // При неизвестном или пустом уровне уровень логирования не меняется
}

