#include <filters/BrightnessFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <algorithm>

FilterResult BrightnessFilter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return FilterResult::failure(FilterError::InvalidImage, "Изображение не загружено");
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    // Валидация размеров изображения
    if (width <= 0 || height <= 0)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "factor=" + std::to_string(factor_);
        return FilterResult::failure(FilterError::InvalidSize, 
                                     "Размер изображения должен быть больше нуля", ctx);
    }

    if (channels != 3 && channels != 4)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "factor=" + std::to_string(factor_);
        return FilterResult::failure(FilterError::InvalidChannels, 
                                     "Ожидается 3 канала (RGB) или 4 канала (RGBA), получено: " + std::to_string(channels),
                                     ctx);
    }

    // Валидация параметра фильтра
    if (factor_ <= 0.0)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        ctx.filter_params = "factor=" + std::to_string(factor_);
        return FilterResult::failure(FilterError::InvalidFactor,
                                     "Коэффициент яркости должен быть больше нуля, получено: " + std::to_string(factor_),
                                     ctx);
    }

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



