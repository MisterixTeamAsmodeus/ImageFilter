#include <cli/ProgressDisplay.h>
#include <utils/Logger.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>

std::string ProgressDisplay::formatTime(std::chrono::seconds seconds)
{
    const auto hours = seconds.count() / 3600;
    const auto minutes = (seconds.count() % 3600) / 60;
    const auto secs = seconds.count() % 60;

    std::ostringstream oss;
    if (hours > 0)
    {
        oss << hours << "ч " << minutes << "м " << secs << "с";
    }
    else if (minutes > 0)
    {
        oss << minutes << "м " << secs << "с";
    }
    else
    {
        oss << secs << "с";
    }
    return oss.str();
}

void ProgressDisplay::displayProgress(const ProgressInfo& info)
{
    if (Logger::isQuiet())
    {
        return;
    }
    
    std::cout << "\r[";
    std::cout << std::setw(3) << std::fixed << std::setprecision(0) << info.percentage << "%] ";
    std::cout << "[" << info.current << "/" << info.total << "] ";
    
    // Отображаем имя файла (только последнюю часть пути для компактности)
    std::filesystem::path file_path(info.current_file);
    std::string filename = file_path.filename().string();
    if (filename.length() > 40)
    {
        filename = filename.substr(0, 37) + "...";
    }
    std::cout << filename;
    
    // Отображаем время и ETA
    if (info.elapsed_time.count() > 0)
    {
        std::cout << " | Время: " << formatTime(info.elapsed_time);
        
        if (info.estimated_remaining.count() > 0 && info.current < info.total)
        {
            std::cout << " | Осталось: " << formatTime(info.estimated_remaining);
        }
        
        if (info.files_per_second > 0.0)
        {
            std::cout << " | " << std::fixed << std::setprecision(1) 
                      << info.files_per_second << " файл/с";
        }
    }
    
    if (info.current == info.total)
    {
        std::cout << std::endl;
    }
    else
    {
        std::cout.flush();
    }
}

