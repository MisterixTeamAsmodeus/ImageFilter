#include <filters/VignetteFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <algorithm>

bool VignetteFilter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return false;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    if (channels != 3 || strength_ < 0.0 || strength_ > 1.0)
    {
        return false;
    }

    auto* data = image.getData();

    // Центр изображения
    const auto center_x = width / 2.0;
    const auto center_y = height / 2.0;

    // Максимальное расстояние от центра до угла
    const auto max_distance = std::sqrt(center_x * center_x + center_y * center_y);

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data, center_x, center_y, max_distance, this](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * width * channels;

                for (int x = 0; x < width; ++x)
                {
                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * channels;

                    // Вычисляем расстояние от центра
                    const auto dx = x - center_x;
                    const auto dy = y - center_y;
                    const auto distance = std::sqrt(dx * dx + dy * dy);

                    // Вычисляем коэффициент виньетирования (1.0 в центре, уменьшается к краям)
                    const auto vignette_factor = 1.0 - (distance / max_distance) * strength_;
                    const auto factor = static_cast<int>(vignette_factor * 65536);

                    for (int c = 0; c < channels; ++c)
                    {
                        const auto old_value = static_cast<int>(data[pixel_offset + c]);
                        const auto new_value = (old_value * factor) >> 16;
                        data[pixel_offset + c] = static_cast<uint8_t>(std::max(0, std::min(255, new_value)));
                    }
                }
            }
        }
    );

    return true;
}


