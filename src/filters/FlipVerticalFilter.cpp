#include <filters/FlipVerticalFilter.h>
#include <ImageProcessor.h>
#include <utils/FilterResult.h>
#include <algorithm>

FilterResult FlipVerticalFilter::apply(ImageProcessor& image)
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
    const auto row_size = static_cast<size_t>(width) * static_cast<size_t>(channels);

    // Меняем местами строки
    for (int y = 0; y < height / 2; ++y)
    {
        const auto top_row_offset = static_cast<size_t>(y) * row_size;
        const auto bottom_row_offset = static_cast<size_t>(height - 1 - y) * row_size;

        // Меняем местами всю строку
        for (size_t x = 0; x < row_size; ++x)
        {
            std::swap(data[top_row_offset + x], data[bottom_row_offset + x]);
        }
    }

    return FilterResult::success();
}

std::string FlipVerticalFilter::getName() const
{
    return "flip_v";
}

std::string FlipVerticalFilter::getDescription() const
{
    return "Вертикальное отражение";
}

std::string FlipVerticalFilter::getCategory() const
{
    return "Геометрический";
}



