#include <filters/OutlineFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/BorderHandler.h>
#include <vector>

FilterResult OutlineFilter::apply(ImageProcessor& image)
{
    if (!image.isValid())
    {
        return FilterResult::failure(FilterError::InvalidImage, "Изображение не загружено");
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    // Валидация размеров изображения
    if (width <= 0 || height <= 0)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        return FilterResult::failure(FilterError::InvalidSize,
                                     "Размер изображения должен быть больше нуля", ctx);
    }

    if (channels != 3 && channels != 4)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        return FilterResult::failure(FilterError::InvalidChannels, 
                                     "Ожидается 3 канала (RGB) или 4 канала (RGBA), получено: " + std::to_string(channels),
                                     ctx);
    }

    const auto* input_data = image.getData();

    // Сначала преобразуем в градации серого
    const auto grayscale_size = static_cast<size_t>(width) * static_cast<size_t>(height);
    
    // Создаем буфер для градаций серого
    std::vector<uint8_t> grayscale(grayscale_size);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            constexpr int R_COEFF = 19595;
            constexpr int G_COEFF = 38470;
            constexpr int B_COEFF = 7471;

            const auto pixel_offset = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels);
            const auto r = static_cast<int>(input_data[pixel_offset + 0]);
            const auto g = static_cast<int>(input_data[pixel_offset + 1]);
            const auto b = static_cast<int>(input_data[pixel_offset + 2]);
            grayscale[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)] = static_cast<uint8_t>((R_COEFF * r + G_COEFF * g + B_COEFF *
                b) >> 16);
        }
    }
    std::vector<int> laplacian_result(static_cast<size_t>(width) * static_cast<size_t>(height));
    ParallelImageProcessor::processRowsParallel(
        height,
        width,
        [width, height, &grayscale, &laplacian_result, this](int start_row, int end_row)
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

                            // Обработка границ с использованием BorderHandler
                            const auto clamped_x = border_handler_.getX(px, width);
                            const auto clamped_y = border_handler_.getY(py, height);

                            const auto pixel_value = static_cast<int>(grayscale[static_cast<size_t>(clamped_y) * static_cast<size_t>(width) +
                                static_cast<size_t>(clamped_x)]);
                            sum += pixel_value * laplacian_kernel[ky + 1][kx + 1];
                        }
                    }

                    laplacian_result[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)] = sum;
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
                        const auto pixel_offset = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels);
                        const auto laplacian = laplacian_result[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)];
                        // Защита от переполнения при умножении
                        const int64_t numerator = static_cast<int64_t>(laplacian - min_val) * 255;
                        const auto normalized = static_cast<uint8_t>(numerator / range);

                        data[pixel_offset + 0] = normalized;
                        data[pixel_offset + 1] = normalized;
                        data[pixel_offset + 2] = normalized;
                    }
                }
            }
        );
    }

    return FilterResult::success();
}

std::string OutlineFilter::getName() const
{
    return "outline";
}

std::string OutlineFilter::getDescription() const
{
    return "Выделение контуров";
}

std::string OutlineFilter::getCategory() const
{
    return "Края и детали";
}



