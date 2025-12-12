#include <filters/EmbossFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <algorithm>
#include <vector>

bool EmbossFilter::apply(ImageProcessor& image)
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

    const auto* input_data = image.getData();
    std::vector<uint8_t> result(static_cast<size_t>(width) * height * channels);


    ParallelImageProcessor::processRowsParallel(
        height,
        [width, height, channels, input_data, &result](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    for (int c = 0; c < channels; ++c)
                    {
                        int sum = 0;

                        // Применяем ядро рельефа
                        for (int ky = -1; ky <= 1; ++ky)
                        {
                            for (int kx = -1; kx <= 1; ++kx)
                            {
                                // Ядро рельефа (emboss kernel)
                                constexpr int emboss_kernel[3][3] = {
                                    {-2, -1, 0},
                                    {-1, 1, 1},
                                    {0, 1, 2}
                                };
                                const auto px = x + kx;
                                const auto py = y + ky;

                                // Обработка границ: используем отражение
                                int clamped_x = px;
                                if (clamped_x < 0) clamped_x = -clamped_x;
                                else if (clamped_x >= width) clamped_x = 2 * width - clamped_x - 1;

                                int clamped_y = py;
                                if (clamped_y < 0) clamped_y = -clamped_y;
                                else if (clamped_y >= height) clamped_y = 2 * height - clamped_y - 1;

                                const auto pixel_offset = (static_cast<size_t>(clamped_y) * width + clamped_x) *
                                    channels + c;
                                sum += static_cast<int>(input_data[pixel_offset]) * emboss_kernel[ky + 1][kx + 1];
                            }
                        }

                        // Добавляем 128 для смещения в средний диапазон
                        const auto value = sum + 128;
                        const auto pixel_offset = (static_cast<size_t>(y) * width + x) * channels + c;
                        result[pixel_offset] = static_cast<uint8_t>(std::max(0, std::min(255, value)));
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


