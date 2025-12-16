#include <filters/ThresholdFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidator.h>
#include <utils/FilterValidationHelper.h>
#include <utils/ColorConversionUtils.h>

FilterResult ThresholdFilter::apply(ImageProcessor& image)
{
    // Валидация параметра фильтра
    auto threshold_result = FilterValidator::validateThreshold(threshold_);
    
    // Валидация изображения и параметра с автоматическим добавлением контекста
    auto validation_result = FilterValidationHelper::validateImageAndParam(
        image, threshold_result, "threshold", threshold_);
    if (validation_result.hasError())
    {
        return validation_result;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();
    auto* data = image.getData();

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data, threshold = threshold_](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);

                for (int x = 0; x < width; ++x)
                {
                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels);

                    const auto r = static_cast<int>(data[pixel_offset + 0]);
                    const auto g = static_cast<int>(data[pixel_offset + 1]);
                    const auto b = static_cast<int>(data[pixel_offset + 2]);

                    // Вычисляем яркость используя общую утилиту
                    const auto gray = ColorConversionUtils::rgbToGrayscaleInt(r, g, b);

                    // Применяем порог
                    const auto value = (gray >= threshold) ? 255 : 0;

                    // Применяем порог только к цветовым каналам
                    // Альфа-канал сохраняется без изменений
                    data[pixel_offset + 0] = static_cast<uint8_t>(value);
                    data[pixel_offset + 1] = static_cast<uint8_t>(value);
                    data[pixel_offset + 2] = static_cast<uint8_t>(value);
                    // Альфа-канал (pixel_offset + 3) не изменяется, если channels == 4
                }
            }
        }
    );

    return FilterResult::success();
}

std::string ThresholdFilter::getName() const
{
    return "threshold";
}

std::string ThresholdFilter::getDescription() const
{
    return "Пороговая бинаризация";
}

std::string ThresholdFilter::getCategory() const
{
    return "Стилистический";
}



