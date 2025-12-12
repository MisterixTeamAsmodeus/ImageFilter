/**
 * @file example_parallel_processing.cpp
 * @brief Пример использования параллельной обработки изображений
 * 
 * Этот пример демонстрирует:
 * - Использование ParallelImageProcessor для распараллеливания обработки
 * - Обработку изображения построчно в нескольких потоках
 * - Адаптивный выбор режима обработки (последовательный/параллельный)
 * 
 * @example example_parallel_processing.cpp
 */

#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/ThreadPool.h>
#include <utils/Logger.h>
#include <iostream>
#include <vector>
#include <algorithm>

/**
 * @brief Пример функции обработки изображения с использованием параллелизма
 * 
 * Демонстрирует применение фильтра яркости к изображению с распараллеливанием
 * обработки по строкам.
 */
void applyBrightnessParallel(ImageProcessor& image, float factor)
{
    int width = image.getWidth();
    int height = image.getHeight();
    int channels = image.getChannels();
    uint8_t* data = image.getData();
    
    // Функция обработки диапазона строк
    auto processRows = [&](int start_row, int end_row) {
        for (int y = start_row; y < end_row; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int index = (y * width + x) * channels;
                for (int c = 0; c < channels; ++c)
                {
                    float value = static_cast<float>(data[index + c]) * factor;
                    value = std::clamp(value, 0.0f, 255.0f);
                    data[index + c] = static_cast<uint8_t>(value);
                }
            }
        }
    };
    
    // Используем ParallelImageProcessor для распараллеливания
    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        processRows
    );
}

int main()
{
    // Создаем экземпляр ImageProcessor и загружаем изображение
    ImageProcessor processor;
    auto result = processor.loadFromFile("input.jpg");
    if (!result.isSuccess())
    {
        Logger::error("Ошибка загрузки изображения: " + result.getMessage());
        return 1;
    }
    
    int width = processor.getWidth();
    int height = processor.getHeight();
    
    Logger::info("Изображение загружено: " + std::to_string(width) + "x" + std::to_string(height));
    
    // Проверяем, рекомендуется ли параллельная обработка
    bool should_parallel = ParallelImageProcessor::shouldUseParallelProcessing(width, height);
    Logger::info("Рекомендуется параллельная обработка: " + std::string(should_parallel ? "да" : "нет"));
    
    // Получаем оптимальное количество потоков
    int optimal_threads = ParallelImageProcessor::getOptimalThreadCount();
    Logger::info("Оптимальное количество потоков: " + std::to_string(optimal_threads));
    
    // Применяем параллельную обработку
    Logger::info("Применение фильтра яркости с параллельной обработкой...");
    applyBrightnessParallel(processor, 1.2f);
    
    // Сохраняем результат
    result = processor.saveToFile("output_parallel.jpg");
    if (!result.isSuccess())
    {
        Logger::error("Ошибка сохранения изображения: " + result.getMessage());
        return 1;
    }
    
    Logger::info("Изображение успешно обработано и сохранено");
    return 0;
}

