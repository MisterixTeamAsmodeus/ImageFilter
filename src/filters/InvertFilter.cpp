#include <filters/InvertFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>

FilterResult InvertFilter::apply(ImageProcessor& image)
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
        return FilterResult::failure(FilterError::InvalidSize,
                                     "Размер изображения должен быть больше нуля", ctx);
    }

    if (channels != 3 && channels != 4)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        return FilterResult::failure(FilterError::InvalidChannels, 
                                     "Ожидается 3 канала (RGB) или 4 канала (RGBA), получено: " + std::to_string(channels),
                                     ctx);
    }

    auto* data = image.getData();

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data](const int start_row, const int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);

                for (int x = 0; x < width; ++x)
                {
                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels);

                    // Инвертируем только цветовые каналы (RGB)
                    // Альфа-канал сохраняется без изменений
                    data[pixel_offset + 0] = static_cast<uint8_t>(255 - data[pixel_offset + 0]);
                    data[pixel_offset + 1] = static_cast<uint8_t>(255 - data[pixel_offset + 1]);
                    data[pixel_offset + 2] = static_cast<uint8_t>(255 - data[pixel_offset + 2]);
                    // Альфа-канал (pixel_offset + 3) не изменяется, если channels == 4
                }
            }
        }
    );

    return FilterResult::success();
}

std::string InvertFilter::getName() const
{
    return "invert";
}

std::string InvertFilter::getDescription() const
{
    return "Инверсия цветов";
}

std::string InvertFilter::getCategory() const
{
    return "Цветовой";
}



