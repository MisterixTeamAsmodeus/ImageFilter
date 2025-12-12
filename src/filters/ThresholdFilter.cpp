#include <filters/ThresholdFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>

bool ThresholdFilter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return false;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    if (channels != 3 || threshold_ < 0 || threshold_ > 255)
    {
        return false;
    }

    auto* data = image.getData();

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data, this](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                const auto row_offset = static_cast<size_t>(y) * width * channels;

                for (int x = 0; x < width; ++x)
                {
                    // Коэффициенты для преобразования RGB в градации серого
                    constexpr int R_COEFF = 19595; // 0.299 * 65536
                    constexpr int G_COEFF = 38470; // 0.587 * 65536
                    constexpr int B_COEFF = 7471; // 0.114 * 65536

                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * channels;

                    const auto r = static_cast<int>(data[pixel_offset + 0]);
                    const auto g = static_cast<int>(data[pixel_offset + 1]);
                    const auto b = static_cast<int>(data[pixel_offset + 2]);

                    // Вычисляем яркость
                    const auto gray = (R_COEFF * r + G_COEFF * g + B_COEFF * b) >> 16;

                    // Применяем порог
                    const auto value = (gray >= threshold_) ? 255 : 0;

                    data[pixel_offset + 0] = static_cast<uint8_t>(value);
                    data[pixel_offset + 1] = static_cast<uint8_t>(value);
                    data[pixel_offset + 2] = static_cast<uint8_t>(value);
                }
            }
        }
    );

    return true;
}


