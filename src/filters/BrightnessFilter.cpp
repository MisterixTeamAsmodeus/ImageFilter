#include <filters/BrightnessFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidator.h>
#include <utils/FilterValidationHelper.h>
#include <algorithm>

FilterResult BrightnessFilter::apply(ImageProcessor& image)
{
    // Валидация параметра фильтра
    auto factor_result = FilterValidator::validateFactor(factor_, 0.0);
    
    // Валидация изображения и параметра с автоматическим добавлением контекста
    auto validation_result = FilterValidationHelper::validateImageAndParam(
        image, factor_result, "factor", factor_);
    if (validation_result.hasError())
    {
        return validation_result;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();
    auto* data = image.getData();
    const auto factor = static_cast<int>(factor_ * 65536); // Масштабируем для целочисленной арифметики
    constexpr int color_channels = 3; // Обрабатываем только RGB каналы

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data, factor](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);

                for (int x = 0; x < width; ++x)
                {
                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels);

                    // Применяем яркость только к цветовым каналам (RGB)
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

std::string BrightnessFilter::getName() const
{
    return "brightness";
}

std::string BrightnessFilter::getDescription() const
{
    return "Изменение яркости";
}

std::string BrightnessFilter::getCategory() const
{
    return "Цветовой";
}

bool BrightnessFilter::supportsInPlace() const noexcept
{
    return true;
}



