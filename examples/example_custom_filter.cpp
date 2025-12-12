/**
 * @file example_custom_filter.cpp
 * @brief Пример создания пользовательского фильтра
 * 
 * Этот пример демонстрирует:
 * - Создание пользовательского фильтра, наследующегося от IFilter
 * - Реализацию метода apply()
 * - Реализацию методов getName(), getDescription(), getCategory()
 * - Использование FilterResult для обработки ошибок
 * 
 * @example example_custom_filter.cpp
 */

#include <ImageProcessor.h>
#include <filters/IFilter.h>
#include <utils/FilterResult.h>
#include <utils/ErrorCodes.h>
#include <utils/Logger.h>
#include <algorithm>
#include <cmath>

/**
 * @brief Пользовательский фильтр для создания эффекта "теплого тона"
 * 
 * Этот фильтр увеличивает красный и желтый каналы, создавая эффект
 * "теплого" освещения на изображении.
 */
class WarmToneFilter : public IFilter
{
public:
    /**
     * @brief Применяет фильтр к изображению
     * @param image Обрабатываемое изображение
     * @return FilterResult с результатом операции
     */
    FilterResult apply(ImageProcessor& image) override
    {
        if (!image.isValid())
        {
            return FilterResult::error(ErrorCodes::INVALID_IMAGE, "Изображение не загружено");
        }
        
        int width = image.getWidth();
        int height = image.getHeight();
        int channels = image.getChannels();
        uint8_t* data = image.getData();
        
        // Коэффициенты для создания теплого тона
        const float red_factor = 1.1f;    // Увеличиваем красный
        const float green_factor = 1.05f; // Слегка увеличиваем зеленый
        const float blue_factor = 0.95f;  // Уменьшаем синий
        
        // Обрабатываем каждый пиксель
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int index = (y * width + x) * channels;
                
                // Применяем коэффициенты к RGB каналам
                float r = static_cast<float>(data[index + 0]) * red_factor;
                float g = static_cast<float>(data[index + 1]) * green_factor;
                float b = static_cast<float>(data[index + 2]) * blue_factor;
                
                // Ограничиваем значения диапазоном [0, 255]
                data[index + 0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
                data[index + 1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
                data[index + 2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
                
                // Альфа-канал не изменяем, если он есть
            }
        }
        
        return FilterResult::success();
    }
    
    /**
     * @brief Возвращает имя фильтра
     * @return Строка с именем фильтра
     */
    std::string getName() const override
    {
        return "warm_tone";
    }
    
    /**
     * @brief Возвращает описание фильтра
     * @return Строка с описанием фильтра
     */
    std::string getDescription() const override
    {
        return "Создает эффект теплого освещения, увеличивая красный и желтый каналы";
    }
    
    /**
     * @brief Возвращает категорию фильтра
     * @return Строка с категорией фильтра
     */
    std::string getCategory() const override
    {
        return "Цветовой";
    }
    
    /**
     * @brief Проверяет, может ли фильтр работать in-place
     * @return true, так как фильтр может работать in-place
     */
    bool supportsInPlace() const noexcept override
    {
        return true;
    }
};

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
    
    // Создаем пользовательский фильтр
    WarmToneFilter warm_filter;
    
    Logger::info("Применение пользовательского фильтра: " + warm_filter.getName());
    Logger::info("Описание: " + warm_filter.getDescription());
    Logger::info("Категория: " + warm_filter.getCategory());
    
    // Применяем фильтр
    result = warm_filter.apply(processor);
    if (!result.isSuccess())
    {
        Logger::error("Ошибка применения фильтра: " + result.getMessage());
        return 1;
    }
    
    // Сохраняем результат
    result = processor.saveToFile("output_warm_tone.jpg");
    if (!result.isSuccess())
    {
        Logger::error("Ошибка сохранения изображения: " + result.getMessage());
        return 1;
    }
    
    Logger::info("Изображение успешно обработано и сохранено");
    return 0;
}

