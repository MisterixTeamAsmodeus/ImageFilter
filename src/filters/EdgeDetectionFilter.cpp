#include <filters/EdgeDetectionFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <cmath>
#include <vector>

bool EdgeDetectionFilter::apply(ImageProcessor& image)
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

    // Сначала преобразуем в градации серого
    std::vector<uint8_t> grayscale(static_cast<size_t>(width) * height);

    // Коэффициенты для преобразования RGB в градации серого

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            constexpr int B_COEFF = 7471;
            constexpr int G_COEFF = 38470;
            constexpr int R_COEFF = 19595;
            const auto pixel_offset = (static_cast<size_t>(y) * width + x) * channels;
            const auto r = static_cast<int>(input_data[pixel_offset + 0]);
            const auto g = static_cast<int>(input_data[pixel_offset + 1]);
            const auto b = static_cast<int>(input_data[pixel_offset + 2]);
            grayscale[static_cast<size_t>(y) * width + x] = static_cast<uint8_t>((R_COEFF * r + G_COEFF * g + B_COEFF *
                b) >> 16);
        }
    }


    std::vector<int> gradient_magnitude(static_cast<size_t>(width) * height);

    // Применяем оператор Собеля
    ParallelImageProcessor::processRowsParallel(
        height,
        [width, height, &grayscale, &gradient_magnitude](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    int gx = 0;
                    int gy = 0;

                    // Применяем ядра Собеля
                    for (int ky = -1; ky <= 1; ++ky)
                    {
                        for (int kx = -1; kx <= 1; ++kx)
                        {
                            // Ядра Собеля для детекции краёв
                            // Gx (горизонтальное)
                            constexpr int sobel_x[3][3] = {
                                {-1, 0, 1},
                                {-2, 0, 2},
                                {-1, 0, 1}
                            };

                            // Gy (вертикальное)
                            constexpr int sobel_y[3][3] = {
                                {-1, -2, -1},
                                {0, 0, 0},
                                {1, 2, 1}
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

                            const auto pixel_value = static_cast<int>(grayscale[static_cast<size_t>(clamped_y) * width +
                                clamped_x]);
                            gx += pixel_value * sobel_x[ky + 1][kx + 1];
                            gy += pixel_value * sobel_y[ky + 1][kx + 1];
                        }
                    }

                    // Вычисляем магнитуду градиента
                    gradient_magnitude[static_cast<size_t>(y) * width + x] = static_cast<int>(std::sqrt(
                        gx * gx + gy * gy));
                }
            }
        }
    );

    // Нормализуем и применяем к изображению
    int max_gradient = 0;
    for (int val : gradient_magnitude)
    {
        if (val > max_gradient)
        {
            max_gradient = val;
        }
    }

    auto* data = image.getData();
    if (max_gradient > 0)
    {
        ParallelImageProcessor::processRowsParallel(
            height,
            [width, channels, &gradient_magnitude, data, max_gradient](int start_row, int end_row)
            {
                for (int y = start_row; y < end_row; ++y)
                {
                    for (int x = 0; x < width; ++x)
                    {
                        const auto pixel_offset = (static_cast<size_t>(y) * width + x) * channels;
                        const auto gradient = gradient_magnitude[static_cast<size_t>(y) * width + x];
                        const auto normalized = static_cast<uint8_t>((gradient * 255) / max_gradient);

                        data[pixel_offset + 0] = normalized;
                        data[pixel_offset + 1] = normalized;
                        data[pixel_offset + 2] = normalized;
                    }
                }
            }
        );
    }

    return true;
}


