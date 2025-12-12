/**
 * @file example_filter_usage.cpp
 * @brief Пример применения фильтров к изображению
 * 
 * Этот пример демонстрирует:
 * - Создание фильтров через FilterFactory
 * - Применение одного фильтра к изображению
 * - Применение цепочки фильтров
 * - Обработку ошибок при применении фильтров
 * 
 * @example example_filter_usage.cpp
 */

#include <ImageProcessor.h>
#include <filters/GrayscaleFilter.h>
#include <filters/GaussianBlurFilter.h>
#include <filters/SharpenFilter.h>
#include <utils/FilterFactory.h>
#include <utils/Logger.h>
#include <CLI/CLI.hpp>
#include <iostream>
#include <vector>

int main()
{
    // Регистрируем все фильтры в фабрике
    FilterFactory::getInstance().registerAll();
    
    // Создаем экземпляр ImageProcessor и загружаем изображение
    ImageProcessor processor;
    auto result = processor.loadFromFile("input.jpg");
    if (!result.isSuccess())
    {
        Logger::error("Ошибка загрузки изображения: " + result.getMessage());
        return 1;
    }
    
    // Создаем CLI::App для передачи параметров фильтрам
    CLI::App app;
    
    // Пример 1: Применение одного фильтра (Grayscale)
    {
        Logger::info("Пример 1: Применение фильтра Grayscale");
        
        auto filter = FilterFactory::getInstance().create("grayscale", app);
        if (filter)
        {
            result = filter->apply(processor);
            if (result.isSuccess())
            {
                processor.saveToFile("output_grayscale.jpg");
                Logger::info("Фильтр применен успешно");
            }
            else
            {
                Logger::error("Ошибка применения фильтра: " + result.getMessage());
            }
        }
        else
        {
            Logger::error("Не удалось создать фильтр grayscale");
        }
    }
    
    // Перезагружаем изображение для следующего примера
    processor.loadFromFile("input.jpg");
    
    // Пример 2: Применение цепочки фильтров
    {
        Logger::info("Пример 2: Применение цепочки фильтров (Grayscale -> Sharpen)");
        
        std::vector<std::string> filter_names = {"grayscale", "sharpen"};
        
        for (const auto& filter_name : filter_names)
        {
            auto filter = FilterFactory::getInstance().create(filter_name, app);
            if (filter)
            {
                result = filter->apply(processor);
                if (!result.isSuccess())
                {
                    Logger::error("Ошибка применения фильтра " + filter_name + ": " + result.getMessage());
                    break;
                }
            }
            else
            {
                Logger::error("Не удалось создать фильтр " + filter_name);
                break;
            }
        }
        
        if (result.isSuccess())
        {
            processor.saveToFile("output_chain.jpg");
            Logger::info("Цепочка фильтров применена успешно");
        }
    }
    
    return 0;
}

