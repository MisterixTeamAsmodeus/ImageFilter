/**
 * @file example_basic_usage.cpp
 * @brief Пример базового использования ImageProcessor для загрузки и сохранения изображений
 * 
 * Этот пример демонстрирует:
 * - Загрузку изображения из файла
 * - Проверку валидности загруженного изображения
 * - Получение информации о размере изображения
 * - Сохранение изображения в файл
 * 
 * @example example_basic_usage.cpp
 */

#include <ImageProcessor.h>
#include <utils/Logger.h>
#include <iostream>

int main()
{
    // Создаем экземпляр ImageProcessor
    ImageProcessor processor;
    
    // Загружаем изображение из файла
    auto result = processor.loadFromFile("input.jpg");
    if (!result.isSuccess())
    {
        Logger::error("Ошибка загрузки изображения: " + result.getMessage());
        return 1;
    }
    
    // Проверяем, что изображение загружено
    if (!processor.isValid())
    {
        Logger::error("Изображение не загружено");
        return 1;
    }
    
    // Получаем информацию о размере изображения
    int width = processor.getWidth();
    int height = processor.getHeight();
    int channels = processor.getChannels();
    
    Logger::info("Изображение загружено:");
    Logger::info("  Размер: " + std::to_string(width) + "x" + std::to_string(height));
    Logger::info("  Каналов: " + std::to_string(channels));
    
    // Устанавливаем качество JPEG (если сохраняем в JPEG)
    processor.setJpegQuality(90);
    
    // Сохраняем изображение
    result = processor.saveToFile("output.jpg");
    if (!result.isSuccess())
    {
        Logger::error("Ошибка сохранения изображения: " + result.getMessage());
        return 1;
    }
    
    Logger::info("Изображение успешно сохранено в output.jpg");
    return 0;
}

