#include <filters/FlipHorizontalFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>

bool FlipHorizontalFilter::apply(ImageProcessor& image)
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
        [width, channels, data](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * width * channels;

                // Отражаем пиксели в строке
                for (int x = 0; x < width / 2; ++x)
                {
                    const auto left_offset = row_offset + static_cast<size_t>(x) * channels;
                    const auto right_offset = row_offset + static_cast<size_t>(width - 1 - x) * channels;

                    // Меняем местами пиксели
                    for (int c = 0; c < channels; ++c)
                    {
                        std::swap(data[left_offset + c], data[right_offset + c]);
                    }
                }
            }
        }
    );

    return true;
}


