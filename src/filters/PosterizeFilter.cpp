#include <filters/PosterizeFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <algorithm>

bool PosterizeFilter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return false;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    if (channels != 3 || levels_ < 2 || levels_ > 256)
    {
        return false;
    }

    auto* data = image.getData();
    const auto step = 256 / levels_;

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data, step, this](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * width * channels;

                for (int x = 0; x < width; ++x)
                {
                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * channels;

                    for (int c = 0; c < channels; ++c)
                    {
                        const auto old_value = static_cast<int>(data[pixel_offset + c]);
                        // Квантуем значение
                        const auto quantized = (old_value / step) * step;
                        // Ограничиваем максимальным значением
                        const auto new_value = std::min(quantized, (levels_ - 1) * step);
                        data[pixel_offset + c] = static_cast<uint8_t>(std::max(0, std::min(255, new_value)));
                    }
                }
            }
        }
    );

    return true;
}


