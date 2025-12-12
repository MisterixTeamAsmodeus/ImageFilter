#include <filters/InvertFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>

bool InvertFilter::apply(ImageProcessor& image)
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

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data](const int start_row, const int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * width * channels;

                for (int x = 0; x < width; ++x)
                {
                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * channels;

                    // Инвертируем каждый канал
                    data[pixel_offset + 0] = static_cast<uint8_t>(255 - data[pixel_offset + 0]);
                    data[pixel_offset + 1] = static_cast<uint8_t>(255 - data[pixel_offset + 1]);
                    data[pixel_offset + 2] = static_cast<uint8_t>(255 - data[pixel_offset + 2]);
                }
            }
        }
    );

    return true;
}


