#include <filters/VignetteFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidator.h>
#include <utils/FilterValidationHelper.h>
#include <utils/LookupTables.h>
#include <algorithm>

FilterResult VignetteFilter::apply(ImageProcessor& image)
{
    // Валидация параметра фильтра
    auto strength_result = FilterValidator::validateIntensity(strength_);
    
    // Валидация изображения и параметра с автоматическим добавлением контекста
    auto validation_result = FilterValidationHelper::validateImageAndParam(
        image, strength_result, "strength", strength_);
    if (validation_result.hasError())
    {
        return validation_result;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();
    auto* data = image.getData();

    // Инициализируем lookup tables
    LookupTables::initialize();

    // Центр изображения
    const auto center_x = width / 2.0;
    const auto center_y = height / 2.0;

    // Максимальное расстояние от центра до угла
    // Используем lookup table для sqrt для оптимизации
    const auto max_distance_squared = static_cast<int>(center_x * center_x + center_y * center_y);
    const auto max_distance = LookupTables::sqrtInt(max_distance_squared);

    constexpr int color_channels = 3; // Обрабатываем только RGB каналы

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data, center_x, center_y, max_distance, strength = strength_](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);

                for (int x = 0; x < width; ++x)
                {
                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels);

                    // Вычисляем расстояние от центра
                    // Используем lookup table для sqrt для оптимизации
                    const auto dx = x - center_x;
                    const auto dy = y - center_y;
                    const auto distance_squared = static_cast<int>(dx * dx + dy * dy);
                    const auto distance = LookupTables::sqrtInt(distance_squared);

                    // Вычисляем коэффициент виньетирования (1.0 в центре, уменьшается к краям)
                    // Защита от деления на ноль
                    double vignette_factor = 1.0;
                    if (max_distance > 0.0)
                    {
                        vignette_factor = 1.0 - (distance / max_distance) * strength;
                        // Ограничиваем диапазон [0.0, 1.0]
                        vignette_factor = std::max(0.0, std::min(1.0, vignette_factor));
                    }
                    const auto factor = static_cast<int>(vignette_factor * 65536);

                    // Применяем виньетирование только к цветовым каналам (RGB)
                    // Альфа-канал сохраняется без изменений
                    for (int c = 0; c < color_channels; ++c)
                    {
                        const auto old_value = static_cast<int>(data[pixel_offset + static_cast<size_t>(c)]);
                        const auto new_value = (old_value * factor) >> 16;
                        data[pixel_offset + static_cast<size_t>(c)] = static_cast<uint8_t>(std::max(0, std::min(255, new_value)));
                    }
                    // Альфа-канал (pixel_offset + 3) не изменяется, если channels == 4
                }
            }
        }
    );

    return FilterResult::success();
}

std::string VignetteFilter::getName() const
{
    return "vignette";
}

std::string VignetteFilter::getDescription() const
{
    return "Виньетирование";
}

std::string VignetteFilter::getCategory() const
{
    return "Стилистический";
}



