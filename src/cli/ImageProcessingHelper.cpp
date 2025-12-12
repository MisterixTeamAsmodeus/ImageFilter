#include <cli/ImageProcessingHelper.h>
#include <ImageProcessor.h>
#include <filters/IFilter.h>
#include <CLI/CLI.hpp>
#include <utils/Logger.h>
#include <utils/FilterFactory.h>
#include <utils/BufferPool.h>
#include <sstream>
#include <algorithm>

namespace
{
    /**
     * @brief Убирает пробелы в начале и конце строки
     */
    std::string trim(const std::string& str)
    {
        if (str.empty())
        {
            return str;
        }
        
        size_t start = str.find_first_not_of(" \t");
        if (start == std::string::npos)
        {
            return "";
        }
        
        size_t end = str.find_last_not_of(" \t");
        return str.substr(start, end - start + 1);
    }
}

std::vector<std::string> ImageProcessingHelper::parseFilterChain(const std::string& filter_chain)
{
    std::vector<std::string> filters;
    std::stringstream ss(filter_chain);
    std::string filter;
    
    while (std::getline(ss, filter, ','))
    {
        filter = trim(filter);
        if (!filter.empty())
        {
            filters.push_back(filter);
        }
    }
    
    return filters;
}

bool ImageProcessingHelper::processSingleImage(
    const std::string& input_file,
    const std::string& output_file,
    const std::vector<std::string>& filter_names,
    CLI::App& app,
    bool preserve_alpha,
    bool force_rgb,
    int jpeg_quality)
{
    ImageProcessor image;
    
    // Устанавливаем качество JPEG
    if (!image.setJpegQuality(jpeg_quality))
    {
        Logger::error("Ошибка: недопустимое качество JPEG");
        return false;
    }
    
    const auto load_result = image.loadFromFile(input_file, preserve_alpha);
    if (!load_result.isSuccess())
    {
        Logger::error("Ошибка загрузки изображения: " + load_result.getFullMessage());
        return false;
    }
    
    // Принудительное преобразование RGBA в RGB, если запрошено
    if (force_rgb && image.hasAlpha())
    {
        const auto convert_result = image.convertToRGB();
        if (!convert_result.isSuccess())
        {
            Logger::error("Ошибка преобразования RGBA в RGB: " + convert_result.getFullMessage());
            return false;
        }
    }
    
    // Создаем пул буферов для оптимизации использования памяти
    // Используем один пул для всей цепочки фильтров
    BufferPool buffer_pool;
    auto& factory = FilterFactory::getInstance();
    factory.setBufferPool(&buffer_pool);
    
    // Применяем фильтры по очереди
    for (const auto& filter_name : filter_names)
    {
        auto filter = factory.create(filter_name, app);
        if (!filter)
        {
            return false;
        }
        
        const auto result = filter->apply(image);
        if (!result.isSuccess())
        {
            Logger::error("Ошибка применения фильтра " + filter_name + ": " + result.getFullMessage());
            return false;
        }
    }
    
    // Определяем, нужно ли сохранять альфа-канал
    bool save_alpha = preserve_alpha && image.hasAlpha() && !force_rgb;
    
    const auto save_result = image.saveToFile(output_file, save_alpha);
    if (!save_result.isSuccess())
    {
        Logger::error("Ошибка сохранения изображения: " + save_result.getFullMessage());
        return false;
    }
    
    return true;
}

