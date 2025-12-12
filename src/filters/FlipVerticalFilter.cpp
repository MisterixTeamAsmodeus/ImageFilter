#include <filters/FlipVerticalFilter.h>
#include <ImageProcessor.h>
#include <algorithm>

bool FlipVerticalFilter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return false;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    if (channels != 3)
    {
        return false;
    }

    auto* data = image.getData();
    const auto row_size = static_cast<size_t>(width) * channels;

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

    return true;
}


