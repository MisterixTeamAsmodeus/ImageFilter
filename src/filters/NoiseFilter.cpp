#include <filters/NoiseFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <algorithm>
#include <random>

bool NoiseFilter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return false;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    if (channels != 3 || intensity_ < 0.0 || intensity_ > 1.0)
    {
        return false;
    }

    auto* data = image.getData();
    const auto max_noise = static_cast<int>(intensity_ * 255);

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data, max_noise](int start_row, int end_row)
        {
            // Генератор случайных чисел для каждого потока
            thread_local std::mt19937 local_gen(std::random_device{}());
            std::uniform_int_distribution local_dist(-max_noise, max_noise);

            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * width * channels;

                for (int x = 0; x < width; ++x)
                {
                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * channels;

                    for (int c = 0; c < channels; ++c)
                    {
                        const auto old_value = static_cast<int>(data[pixel_offset + c]);
                        const auto noise = local_dist(local_gen);
                        const auto new_value = old_value + noise;
                        data[pixel_offset + c] = static_cast<uint8_t>(std::max(0, std::min(255, new_value)));
                    }
                }
            }
        }
    );

    return true;
}
