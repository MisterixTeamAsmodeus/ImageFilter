#include <filters/BoxBlurFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <algorithm>
#include <vector>

bool BoxBlurFilter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return false;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    if (channels != 3 || radius_ <= 0)
    {
        return false;
    }

    const auto* input_data = image.getData();
    const auto kernel_size = 2 * radius_ + 1;
    const auto kernel_weight = 65536 / kernel_size; // Масштабированный вес для целочисленной арифметики

    // Применяем separable kernel: сначала по горизонтали
    std::vector<uint8_t> horizontal_result(static_cast<size_t>(width) * height * channels);

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, input_data, &horizontal_result, this, kernel_weight](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * width * channels;

                for (int x = 0; x < width; ++x)
                {
                    for (int c = 0; c < channels; ++c)
                    {
                        int64_t sum = 0;

                        // Применяем ядро по горизонтали
                        for (int kx = -radius_; kx <= radius_; ++kx)
                        {
                            const auto sample_x = x + kx;

                            // Обработка границ: используем отражение
                            int clamped_x = sample_x;
                            if (clamped_x < 0) clamped_x = -clamped_x;
                            else if (clamped_x >= width) clamped_x = 2 * width - clamped_x - 1;

                            const auto pixel_index = row_offset + static_cast<size_t>(clamped_x) * channels + c;
                            sum += static_cast<int>(input_data[pixel_index]);
                        }

                        const auto result_value = static_cast<int>((sum * kernel_weight) >> 16);
                        const auto result_index = row_offset + static_cast<size_t>(x) * channels + c;
                        horizontal_result[result_index] = static_cast<uint8_t>(
                            std::max(0, std::min(255, result_value)));
                    }
                }
            }
        }
    );

    // Применяем ядро по вертикали
    std::vector<uint8_t> final_result(static_cast<size_t>(width) * height * channels);

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, height, channels, &horizontal_result, &final_result, this, kernel_weight](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * width * channels;

                for (int x = 0; x < width; ++x)
                {
                    for (int c = 0; c < channels; ++c)
                    {
                        int64_t sum = 0;

                        // Применяем ядро по вертикали
                        for (int ky = -radius_; ky <= radius_; ++ky)
                        {
                            const auto sample_y = y + ky;

                            // Обработка границ: используем отражение
                            int clamped_y = sample_y;
                            if (clamped_y < 0) clamped_y = -clamped_y;
                            else if (clamped_y >= height) clamped_y = 2 * height - clamped_y - 1;

                            const auto pixel_index = (static_cast<size_t>(clamped_y) * width + x) * channels + c;
                            sum += static_cast<int>(horizontal_result[pixel_index]);
                        }

                        const auto result_value = static_cast<int>((sum * kernel_weight) >> 16);
                        const auto result_index = row_offset + static_cast<size_t>(x) * channels + c;
                        final_result[result_index] = static_cast<uint8_t>(std::max(0, std::min(255, result_value)));
                    }
                }
            }
        }
    );

    // Копируем результат обратно
    auto* data = image.getData();
    std::ranges::copy(final_result, data);

    return true;
}


