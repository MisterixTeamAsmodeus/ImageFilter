#include <filters/PosterizeFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/FilterValidator.h>
#include <utils/FilterValidationHelper.h>
#include <algorithm>

FilterResult PosterizeFilter::apply(ImageProcessor& image)
{
    // Валидация параметра фильтра
    auto levels_result = FilterValidator::validateRange(levels_, 2, 256, "levels");
    
    // Валидация изображения и параметра с автоматическим добавлением контекста
    auto validation_result = FilterValidationHelper::validateImageAndParam(
        image, levels_result, "levels", levels_);
    if (validation_result.hasError())
    {
        return validation_result;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();
    auto* data = image.getData();
    const auto step = 256 / levels_;

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data, step, levels = levels_](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);

                for (int x = 0; x < width; ++x)
                {
                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels);

                    // Обрабатываем все каналы, включая альфа-канал (если есть)
                    // Постеризация может применяться и к альфа-каналу
                    for (int c = 0; c < channels; ++c)
                    {
                        const auto old_value = static_cast<int>(data[pixel_offset + static_cast<size_t>(c)]);
                        // Квантуем значение
                        const auto quantized = (old_value / step) * step;
                        // Ограничиваем максимальным значением
                        const auto new_value = std::min(quantized, (levels - 1) * step);
                        data[pixel_offset + static_cast<size_t>(c)] = static_cast<uint8_t>(std::max(0, std::min(255, new_value)));
                    }
                }
            }
        }
    );

    return FilterResult::success();
}

std::string PosterizeFilter::getName() const
{
    return "posterize";
}

std::string PosterizeFilter::getDescription() const
{
    return "Постеризация";
}

std::string PosterizeFilter::getCategory() const
{
    return "Стилистический";
}



