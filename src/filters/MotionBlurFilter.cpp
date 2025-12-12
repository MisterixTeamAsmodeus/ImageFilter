#include <filters/MotionBlurFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <algorithm>
#include <vector>
#include <numbers>

bool MotionBlurFilter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return false;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    if (channels != 3 || length_ <= 0)
    {
        return false;
    }

    const auto* input_data = image.getData();
    std::vector<uint8_t> result(static_cast<size_t>(width) * height * channels);

    // Преобразуем угол в радианы
    const auto angle_rad = angle_ * std::numbers::pi / 180.0;
    const auto dx = std::cos(angle_rad);
    const auto dy = std::sin(angle_rad);

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, height, channels, input_data, &result, this, dx, dy](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    for (int c = 0; c < channels; ++c)
                    {
                        int64_t sum = 0;
                        int count = 0;

                        // Собираем пиксели вдоль линии движения
                        for (int i = -length_ / 2; i <= length_ / 2; ++i)
                        {
                            const auto sample_x = static_cast<int>(x + i * dx);
                            const auto sample_y = static_cast<int>(y + i * dy);

                            // Обработка границ: используем отражение
                            int clamped_x = sample_x;
                            if (clamped_x < 0) clamped_x = -clamped_x;
                            else if (clamped_x >= width) clamped_x = 2 * width - clamped_x - 1;

                            int clamped_y = sample_y;
                            if (clamped_y < 0) clamped_y = -clamped_y;
                            else if (clamped_y >= height) clamped_y = 2 * height - clamped_y - 1;

                            const auto pixel_offset = (static_cast<size_t>(clamped_y) * width + clamped_x) * channels +
                                c;
                            sum += static_cast<int>(input_data[pixel_offset]);
                            ++count;
                        }

                        const auto avg = static_cast<int>(sum / count);
                        const auto pixel_offset = (static_cast<size_t>(y) * width + x) * channels + c;
                        result[pixel_offset] = static_cast<uint8_t>(std::max(0, std::min(255, avg)));
                    }
                }
            }
        }
    );

    // Копируем результат обратно
    auto* data = image.getData();
    std::ranges::copy(result, data);

    return true;
}


