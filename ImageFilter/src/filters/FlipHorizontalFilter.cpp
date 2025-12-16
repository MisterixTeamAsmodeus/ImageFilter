#include <filters/FlipHorizontalFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>

FilterResult FlipHorizontalFilter::apply(ImageProcessor& image)
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
        [width, channels, data](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);

                // Отражаем пиксели в строке
                for (int x = 0; x < width / 2; ++x)
                {
                    const auto left_offset = row_offset + static_cast<size_t>(x) * static_cast<size_t>(channels);
                    const auto right_offset = row_offset + static_cast<size_t>(width - 1 - x) * static_cast<size_t>(channels);

                    // Меняем местами пиксели
                    for (int c = 0; c < channels; ++c)
                    {
                        std::swap(data[left_offset + static_cast<size_t>(c)], data[right_offset + static_cast<size_t>(c)]);
                    }
                }
            }
        }
    );

    return FilterResult::success();
}

std::string FlipHorizontalFilter::getName() const
{
    return "flip_h";
}

std::string FlipHorizontalFilter::getDescription() const
{
    return "Горизонтальное отражение";
}

std::string FlipHorizontalFilter::getCategory() const
{
    return "Геометрический";
}



