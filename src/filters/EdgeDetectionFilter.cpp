#include <filters/EdgeDetectionFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <utils/FilterResult.h>
#include <utils/BorderHandler.h>
#include <utils/LookupTables.h>
#include <cmath>
#include <vector>

FilterResult EdgeDetectionFilter::apply(ImageProcessor& image)
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

    // Инициализируем lookup tables
    LookupTables::initialize();

    const auto* input_data = image.getData();

    // Сначала преобразуем в градации серого
    const auto grayscale_size = static_cast<size_t>(width) * static_cast<size_t>(height);
    
    // Создаем буфер для градаций серого
    std::vector<uint8_t> grayscale(grayscale_size);

    // Коэффициенты для преобразования RGB в градации серого

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            constexpr int B_COEFF = 7471;
            constexpr int G_COEFF = 38470;
            constexpr int R_COEFF = 19595;
            const auto pixel_offset = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels);
            const auto r = static_cast<int>(input_data[pixel_offset + 0]);
            const auto g = static_cast<int>(input_data[pixel_offset + 1]);
            const auto b = static_cast<int>(input_data[pixel_offset + 2]);
            grayscale[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)] = static_cast<uint8_t>((R_COEFF * r + G_COEFF * g + B_COEFF *
                b) >> 16);
        }
    }


    std::vector<int> gradient_magnitude(static_cast<size_t>(width) * static_cast<size_t>(height));

    // Применяем оператор Собеля
    ParallelImageProcessor::processRowsParallel(
        height,
        [width, height, &grayscale, &gradient_magnitude, this](int start_row, int end_row)
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

                            // Обработка границ с использованием BorderHandler
                            const auto clamped_x = border_handler_.getX(px, width);
                            const auto clamped_y = border_handler_.getY(py, height);

                            const auto pixel_value = static_cast<int>(grayscale[static_cast<size_t>(clamped_y) * static_cast<size_t>(width) +
                                static_cast<size_t>(clamped_x)]);
                            gx += pixel_value * sobel_x[ky + 1][kx + 1];
                            gy += pixel_value * sobel_y[ky + 1][kx + 1];
                        }
                    }

                    // Вычисляем магнитуду градиента
                    // Используем lookup table для sqrt для оптимизации
                    const auto gradient_squared = gx * gx + gy * gy;
                    gradient_magnitude[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)] = 
                        static_cast<int>(LookupTables::sqrtInt(gradient_squared));
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
                        const auto pixel_offset = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels);
                        const auto gradient = gradient_magnitude[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)];
                        const auto normalized = static_cast<uint8_t>((gradient * 255) / max_gradient);

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

std::string EdgeDetectionFilter::getName() const
{
    return "edges";
}

std::string EdgeDetectionFilter::getDescription() const
{
    return "Детекция краёв (оператор Собеля)";
}

std::string EdgeDetectionFilter::getCategory() const
{
    return "Края и детали";
}



