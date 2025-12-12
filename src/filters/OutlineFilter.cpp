#include <filters/OutlineFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <vector>

bool OutlineFilter::apply(ImageProcessor& image)
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

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            constexpr int R_COEFF = 19595;
            constexpr int G_COEFF = 38470;
            constexpr int B_COEFF = 7471;

            const auto pixel_offset = (static_cast<size_t>(y) * width + x) * channels;
            const auto r = static_cast<int>(input_data[pixel_offset + 0]);
            const auto g = static_cast<int>(input_data[pixel_offset + 1]);
            const auto b = static_cast<int>(input_data[pixel_offset + 2]);
            grayscale[static_cast<size_t>(y) * width + x] = static_cast<uint8_t>((R_COEFF * r + G_COEFF * g + B_COEFF *
                b) >> 16);
        }
    }
    std::vector<int> laplacian_result(static_cast<size_t>(width) * height);
    ParallelImageProcessor::processRowsParallel(
        height,
        [width, height, &grayscale, &laplacian_result](int start_row, int end_row)
        {
            for (int y = start_row; y < end_row; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    int sum = 0;

                    for (int ky = -1; ky <= 1; ++ky)
                    {
                        for (int kx = -1; kx <= 1; ++kx)
                        {
                            // Ядро Лапласа для детекции контуров
                            constexpr int laplacian_kernel[3][3] = {
                                {0, -1, 0},
                                {-1, 4, -1},
                                {0, -1, 0}
                            };

                            const auto px = x + kx;
                            const auto py = y + ky;

                            int clamped_x = px;
                            if (clamped_x < 0) clamped_x = -clamped_x;
                            else if (clamped_x >= width) clamped_x = 2 * width - clamped_x - 1;

                            int clamped_y = py;
                            if (clamped_y < 0) clamped_y = -clamped_y;
                            else if (clamped_y >= height) clamped_y = 2 * height - clamped_y - 1;

                            const auto pixel_value = static_cast<int>(grayscale[static_cast<size_t>(clamped_y) * width +
                                clamped_x]);
                            sum += pixel_value * laplacian_kernel[ky + 1][kx + 1];
                        }
                    }

                    laplacian_result[static_cast<size_t>(y) * width + x] = sum;
                }
            }
        }
    );

    // Нормализуем и применяем к изображению
    int min_val = laplacian_result[0];
    int max_val = laplacian_result[0];
    for (int val : laplacian_result)
    {
        if (val < min_val) min_val = val;
        if (val > max_val) max_val = val;
    }

    auto* data = image.getData();
    const auto range = max_val - min_val;
    if (range > 0)
    {
        ParallelImageProcessor::processRowsParallel(
            height,
            [width, channels, &laplacian_result, data, min_val, range](int start_row, int end_row)
            {
                for (int y = start_row; y < end_row; ++y)
                {
                    for (int x = 0; x < width; ++x)
                    {
                        const auto pixel_offset = (static_cast<size_t>(y) * width + x) * channels;
                        const auto laplacian = laplacian_result[static_cast<size_t>(y) * width + x];
                        const auto normalized = static_cast<uint8_t>(((laplacian - min_val) * 255) / range);

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


